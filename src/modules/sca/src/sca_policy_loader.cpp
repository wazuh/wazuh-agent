#include <sca_policy_loader.hpp>

#include <dbsync.hpp>
#include <filesystem_wrapper.hpp>
#include <logger.hpp>

#include <algorithm>

SCAPolicyLoader::SCAPolicyLoader(std::shared_ptr<IFileSystemWrapper> fileSystemWrapper,
                                 std::shared_ptr<const configuration::ConfigurationParser> configurationParser,
                                 std::shared_ptr<IDBSync> dBSync,
                                 PolicyLoaderFunc loader)
    : m_fileSystemWrapper(fileSystemWrapper ? std::move(fileSystemWrapper)
                                            : std::make_shared<file_system::FileSystemWrapper>())
    , m_policyLoader(std::move(loader))
    , m_dBSync(std::move(dBSync))
{
    const auto loadPoliciesPathsFromConfig = [this, configurationParser](const std::string& configKey)
    {
        std::vector<std::filesystem::path> policiesPaths;
        const auto policies = configurationParser->GetConfigOrDefault<std::vector<std::string>>({}, "sca", configKey);

        for (const auto& policy : policies)
        {
            if (m_fileSystemWrapper->exists(policy))
            {
                policiesPaths.emplace_back(policy);
            }
            else
            {
                LogWarn("Policy file does not exist: {}", policy);
            }
        }
        return policiesPaths;
    };

    m_customPoliciesPaths = loadPoliciesPathsFromConfig("policies");
    m_disabledPoliciesPaths = loadPoliciesPathsFromConfig("policies_disabled");
}

std::vector<SCAPolicy> SCAPolicyLoader::GetPolicies() const
{
    std::vector<std::filesystem::path> allPolicyPaths;

    allPolicyPaths.insert(allPolicyPaths.end(), m_customPoliciesPaths.begin(), m_customPoliciesPaths.end());

    const auto isDisabled = [this](const std::filesystem::path& path)
    {
        return std::any_of(m_disabledPoliciesPaths.begin(),
                           m_disabledPoliciesPaths.end(),
                           [&](const std::filesystem::path& disabledPath) { return path == disabledPath; });
    };

    std::vector<SCAPolicy> policies;
    for (const auto& path : allPolicyPaths)
    {
        if (!isDisabled(path))
        {
            try
            {
                policies.emplace_back(m_policyLoader(path));
                LogDebug("Loading policy from {}", path.string());
            }
            catch (const std::exception& e)
            {
                LogWarn("Failed to load policy from {}: {}", path.string(), e.what());
            }
        }
    }

    return policies;
}

void SCAPolicyLoader::SyncPoliciesAndReportDelta(const nlohmann::json& data, const CreateEventsFunc& createEvents)
{
    std::unordered_map<std::string, nlohmann::json> modifiedPoliciesMap;
    std::unordered_map<std::string, nlohmann::json> modifiedChecksMap;

    if (data.contains("policies") && data.at("policies").is_array())
    {
        modifiedPoliciesMap = SyncWithDBSync(data["policies"], SCA_POLICY_TABLE_NAME);
    }
    else
    {
        LogError("No policies found in data");
        return;
    }

    if (data.contains("checks") && data.at("checks").is_array())
    {
        modifiedChecksMap = SyncWithDBSync(data["checks"], SCA_CHECK_TABLE_NAME);

        for (auto& check : modifiedChecksMap)
        {
            if (check.second["result"] == MODIFIED)
            {
                try
                {
                    check.second["data"]["new"]["result"] = "Not run";
                    UpdateCheckResult(check.second["data"]["new"]);
                }
                catch (const std::exception& e)
                {
                    LogError("Failed to update check result: {}", e.what());
                }
            }
        }
    }
    else
    {
        LogError("No checks found in data");
        return;
    }

    createEvents(modifiedPoliciesMap, modifiedChecksMap);
}

std::unordered_map<std::string, nlohmann::json> SCAPolicyLoader::SyncWithDBSync(const nlohmann::json& data,
                                                                                const std::string& tableName)
{
    static std::unordered_map<std::string, nlohmann::json> modifiedDataMap;
    modifiedDataMap.clear();

    const auto callback {[](ReturnTypeCallback result, const nlohmann::json& rowData)
                         {
                             if (result != DB_ERROR)
                             {
                                 std::string id;
                                 if (result == MODIFIED && rowData.contains("new") && rowData["new"].is_object())
                                 {
                                     if (rowData["new"].contains("id") && rowData["new"]["id"].is_string())
                                     {
                                         id = rowData["new"]["id"];
                                     }
                                 }
                                 else if ((result == INSERTED || result == DELETED) && rowData.contains("id") &&
                                          rowData["id"].is_string())
                                 {
                                     id = rowData["id"];
                                 }

                                 if (!id.empty())
                                 {
                                     modifiedDataMap[id] = nlohmann::json {{"result", result}, {"data", rowData}};
                                 }
                                 else
                                 {
                                     LogError("Invalid data: {}", rowData.dump());
                                 }
                             }
                             else
                             {
                                 LogError("DB error: {}", rowData.dump());
                             }
                         }};

    DBSyncTxn txn {m_dBSync->handle(), nlohmann::json {tableName}, 0, DBSYNC_QUEUE_SIZE, callback};

    nlohmann::json input;
    input["table"] = tableName;
    input["data"] = data;
    input["options"]["return_old_data"] = true;

    txn.syncTxnRow(input);
    txn.getDeletedRows(callback);

    return modifiedDataMap;
}

void SCAPolicyLoader::UpdateCheckResult(const nlohmann::json& check)
{
    auto updateResultQuery = SyncRowQuery::builder().table(SCA_CHECK_TABLE_NAME).data(check).build();

    const auto callback = [](ReturnTypeCallback, const nlohmann::json&) {
    };

    m_dBSync->syncRow(updateResultQuery.query(), callback);
}
