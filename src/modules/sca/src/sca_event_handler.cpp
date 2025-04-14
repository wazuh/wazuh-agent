#include <sca_event_handler.hpp>

#include <dbsync.hpp>
#include <hashHelper.hpp>
#include <logger.hpp>
#include <stringHelper.hpp>
#include <timeHelper.hpp>

/// @brief Map of operations
static const std::map<ReturnTypeCallback, std::string> OPERATION_MAP {
    {MODIFIED, "update"},
    {DELETED, "delete"},
    {INSERTED, "create"},
    {MAX_ROWS, "max_rows"},
    {DB_ERROR, "db_error"},
    {SELECTED, "selected"},
};

SCAEventHandler::SCAEventHandler(std::string agentUUID,
                                 std::shared_ptr<IDBSync> dBSync,
                                 std::function<int(Message)> pushMessage)
    : m_agentUUID(std::move(agentUUID))
    , m_dBSync(std::move(dBSync))
    , m_pushMessage(std::move(pushMessage)) {};

void SCAEventHandler::CreateEvents(const std::unordered_map<std::string, nlohmann::json>& modifiedPoliciesMap,
                                   const std::unordered_map<std::string, nlohmann::json>& modifiedChecksMap) const
{
    nlohmann::json events = nlohmann::json::array();
    try
    {
        for (const auto& policyEntry : modifiedPoliciesMap)
        {
            const std::string policyId = policyEntry.first;
            const nlohmann::json policyData = policyEntry.second["data"];
            const int policyResult = policyEntry.second["result"];

            const std::vector<nlohmann::json> checksForPolicy = GetChecksForPolicy(policyId);

            for (auto checkData : checksForPolicy)
            {
                const std::string checkId = checkData["id"];

                if (modifiedChecksMap.find(checkId) != modifiedChecksMap.end())
                {
                    continue;
                }

                const nlohmann::json event = {{"policy", policyData}, {"check", checkData}, {"result", policyResult}};
                events.push_back(event);
            }
        }

        for (const auto& checkEntry : modifiedChecksMap)
        {
            nlohmann::json checkData = checkEntry.second["data"];
            int checkResult = checkEntry.second["result"];

            std::string policyId;
            if (checkResult == MODIFIED)
            {
                policyId = checkData["new"]["policy_id"];
            }
            else
            {
                policyId = checkData["policy_id"];
            }

            nlohmann::json policyData = GetPolicyById(policyId);
            if (policyData.empty())
            {
                if (modifiedPoliciesMap.find(policyId) != modifiedPoliciesMap.end())
                {
                    policyData = modifiedPoliciesMap.at(policyId)["data"];
                }
            }
            const nlohmann::json event = {{"policy", policyData}, {"check", checkData}, {"result", checkResult}};
            events.push_back(event);
        }
    }
    catch (const std::exception& e)
    {
        LogError("Failed to create events: {}", e.what());
    }

    for (const auto& event : events)
    {
        ProcessStateful(event);
    }
}

std::vector<nlohmann::json> SCAEventHandler::GetChecksForPolicy(const std::string& policyId) const
{
    std::vector<nlohmann::json> checks;

    const std::string filter = "WHERE policy_id = '" + policyId + "'";
    auto selectQuery = SelectQuery::builder()
                           .table("sca_check")
                           .columnList({"id",
                                        "policy_id",
                                        "name",
                                        "description",
                                        "rationale",
                                        "remediation",
                                        "refs",
                                        "result",
                                        "reason",
                                        "condition",
                                        "compliance",
                                        "rules"})
                           .rowFilter(filter)
                           .build();

    const auto callback = [&checks](ReturnTypeCallback returnTypeCallback, const nlohmann::json& resultData)
    {
        if (returnTypeCallback == SELECTED)
        {
            checks.push_back(resultData);
        }
    };

    m_dBSync->selectRows(selectQuery.query(), callback);

    return checks;
}

nlohmann::json SCAEventHandler::GetPolicyById(const std::string& policyId) const
{
    nlohmann::json policy;

    const std::string filter = "WHERE id = '" + policyId + "'";
    auto selectQuery = SelectQuery::builder()
                           .table("sca_policy")
                           .columnList({"id", "name", "description", "file", "refs"})
                           .rowFilter(filter)
                           .build();

    const auto callback = [&policy](ReturnTypeCallback returnTypeCallback, const nlohmann::json& resultData)
    {
        if (returnTypeCallback == SELECTED)
        {
            policy = resultData;
        }
    };

    m_dBSync->selectRows(selectQuery.query(), callback);

    return policy;
}

void SCAEventHandler::ProcessStateful(const nlohmann::json& event) const
{
    nlohmann::json check;
    nlohmann::json policy;

    try
    {
        if (event.contains("check") && event["check"].is_object())
        {
            if (event["check"].contains("new") && event["check"]["new"].is_object())
            {
                check = event["check"]["new"];
            }
            else
            {
                check = event["check"];
            }
        }

        if (event.contains("policy") && event["policy"].is_object())
        {
            if (event["policy"].contains("new") && event["policy"]["new"].is_object())
            {
                policy = event["policy"]["new"];
            }
            else
            {
                policy = event["policy"];
            }
        }

        if (check.contains("refs") && check["refs"].is_string())
        {
            const std::string refsStr = check["refs"].get<std::string>();
            check["references"] = StringToJsonArray(refsStr);
            check.erase("refs");
        }

        if (check.contains("compliance") && check["compliance"].is_string())
        {
            const std::string refsStr = check["compliance"].get<std::string>();
            check["compliance"] = StringToJsonArray(refsStr);
        }

        if (check.contains("rules") && check["rules"].is_string())
        {
            const std::string refsStr = check["rules"].get<std::string>();
            check["rules"] = StringToJsonArray(refsStr);
        }

        if (policy.contains("refs") && policy["refs"].is_string())
        {
            const std::string refsStr = policy["refs"].get<std::string>();
            policy["references"] = StringToJsonArray(refsStr);
            policy.erase("refs");
        }

        const nlohmann::json jsonEvent = {
            {"policy", policy}, {"check", check}, {"timestamp", Utils::getCurrentISO8601()}};
        const nlohmann::json jsonMetadata = {
            {"id", CalculateHashId(jsonEvent)}, {"operation", OPERATION_MAP.at(event["result"])}, {"module", "sca"}};

        PushMessage(jsonEvent, jsonMetadata);
    }
    catch (const std::exception& e)
    {
        LogError("Error processing stateful event: {}", e.what());
    }
}

std::string SCAEventHandler::CalculateHashId(const nlohmann::json& data) const
{
    const std::string baseId =
        m_agentUUID + ":" + data["policy"]["id"].get<std::string>() + ":" + data["check"]["id"].get<std::string>();

    Utils::HashData hash(Utils::HashType::Sha1);
    hash.update(baseId.c_str(), baseId.size());

    return Utils::asciiToHex(hash.hash());
}

void SCAEventHandler::PushMessage(const nlohmann::json& event, const nlohmann::json& metadata) const
{
    if (!m_pushMessage)
    {
        throw std::runtime_error("Message queue not set, cannot send message.");
    }

    const Message statefulMessage {MessageType::STATEFUL,
                                   metadata["operation"] == "delete" ? "{}"_json : event,
                                   metadata["module"],
                                   "",
                                   metadata.dump()};

    m_pushMessage(statefulMessage);

    LogTrace("Stateful event queued: {}, metadata {}", event.dump(), metadata.dump());
}

nlohmann::json SCAEventHandler::StringToJsonArray(const std::string& input) const
{
    nlohmann::json result = nlohmann::json::array();
    std::istringstream stream(input);
    std::string token;

    while (std::getline(stream, token, ','))
    {
        const size_t start = token.find_first_not_of(" \t");

        const size_t end = token.find_last_not_of(" \t");

        if (start != std::string::npos && end != std::string::npos)
        {
            token = token.substr(start, end - start + 1);
        }
        else
        {
            token = "";
        }

        if (!token.empty())
        {
            result.push_back(token);
        }
    }

    return result;
}
