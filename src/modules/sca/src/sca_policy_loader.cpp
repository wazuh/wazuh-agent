#include <sca_policy_loader.hpp>

#include <filesystem_wrapper.hpp>
#include <logger.hpp>

#include <algorithm>

SCAPolicyLoader::SCAPolicyLoader(std::shared_ptr<IFileSystemWrapper> fileSystemWrapper,
                                 std::shared_ptr<configuration::ConfigurationParser> configurationParser,
                                 PolicyLoaderFunc loader)
    : m_fileSystemWrapper(fileSystemWrapper ? std::move(fileSystemWrapper)
                                            : std::make_shared<file_system::FileSystemWrapper>())
    , m_policyLoader(std::move(loader))
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

    if (m_fileSystemWrapper->exists(m_defaultPolicyPath) && m_fileSystemWrapper->is_directory(m_defaultPolicyPath))
    {
        for (const auto& entry : m_fileSystemWrapper->list_directory(m_defaultPolicyPath))
        {
            if (m_fileSystemWrapper->is_regular_file(entry))
            {
                allPolicyPaths.push_back(entry);
            }
        }
    }

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
