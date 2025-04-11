#include <sca_event_handler.hpp>

SCAEventHandler::SCAEventHandler(std::shared_ptr<IDBSync> dBSync)
    : m_dBSync(std::move(dBSync)) {};

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

            const nlohmann::json policyData = GetPolicyById(policyId);

            const nlohmann::json event = {{"policy", policyData}, {"check", checkData}, {"result", checkResult}};
            events.push_back(event);
        }
    }
    catch (const std::exception& e)
    {
        LogError("Failed to create events: {}", e.what());
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
