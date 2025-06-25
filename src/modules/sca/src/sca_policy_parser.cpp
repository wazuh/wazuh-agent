#include <sca_policy_parser.hpp>

#include <sca_policy.hpp>
#include <sca_policy_check.hpp>

// #include <logger.hpp>

#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <sstream>

namespace
{
    std::string Join(const std::vector<std::string>& elements, const std::string& separator)
    {
        std::ostringstream oss;
        for (size_t i = 0; i < elements.size(); ++i)
        {
            if (i > 0)
            {
                oss << separator;
            }
            oss << elements[i];
        }
        return oss.str();
    }

    // NOLINTNEXTLINE(misc-no-recursion)
    nlohmann::json YamlNodeToJson(const YAML::Node& yamlNode)
    {
        if (yamlNode.IsScalar())
        {
            return yamlNode.as<std::string>();
        }
        else if (yamlNode.IsSequence())
        {
            std::vector<std::string> values;
            for (const auto& item : yamlNode)
            {
                if (item.IsScalar())
                {
                    values.push_back(item.as<std::string>());
                }
                else if (item.IsMap())
                {
                    for (const auto& subitem : item)
                    {
                        const auto key = subitem.first.as<std::string>();
                        const YAML::Node& valNode = subitem.second;
                        if (valNode.IsSequence())
                        {
                            for (const auto& val : valNode)
                            {
                                values.emplace_back(key + ":" + val.as<std::string>());
                            }
                        }
                    }
                }
            }
            return Join(values, ", ");
        }
        else if (yamlNode.IsMap())
        {
            nlohmann::json j;
            for (const auto& kv : yamlNode)
            {
                j[kv.first.as<std::string>()] = YamlNodeToJson(kv.second);
            }
            return j;
        }

        return nullptr;
    }

    void ValidateConditionString(const std::string& value)
    {
        if (!(value == "any" || value == "none" || value == "all"))
        {
            throw std::invalid_argument("Invalid condition: " + value);
        }
    }
} // namespace

// NOLINTNEXTLINE(performance-unnecessary-value-param)
PolicyParser::PolicyParser(const std::filesystem::path& filename, LoadFileFunc loadFileFunc)
    : m_loadFileFunc(loadFileFunc ? std::move(loadFileFunc) : YAML::LoadFile)
{
    try
    {
        if (!IsValidYamlFile(filename.string()))
        {
            throw std::runtime_error("The file does not contain a valid YAML structure.");
        }

        m_node = m_loadFileFunc(filename.string());

        if (auto variables = m_node["variables"]; variables)
        {
            for (const auto& var : variables)
            {
                const auto name = var.first.as<std::string>();
                const auto value = var.second.as<std::string>();
                m_variablesMap[name] = value;
            }
        }
        ReplaceVariablesInNode(m_node);
    }
    catch (const std::exception& e)
    {
        // LogError("Error parsing YAML file: {}", e.what());
    }
}

bool PolicyParser::IsValidYamlFile(const std::filesystem::path& filename) const
{
    try
    {
        const auto mapToValidte = m_loadFileFunc(filename.string());

        if (!mapToValidte.IsMap() && !mapToValidte.IsSequence())
        {
            throw std::runtime_error("The file does not contain a valid YAML structure.");
        }
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

std::unique_ptr<ISCAPolicy> PolicyParser::ParsePolicy(nlohmann::json& policiesAndChecks) const
{
    std::vector<Check> checks;
    Check requirements;

    std::string policyId;

    if (const auto policyNode = m_node["policy"]; policyNode)
    {
        try
        {
            policyId = policyNode["id"].as<std::string>();
            policiesAndChecks["policies"].push_back(YamlNodeToJson(policyNode));

            // LogDebug("Policy parsed.");
        }
        catch (const YAML::Exception& e)
        {
            // LogError("Failed to parse policy. Skipping it. Error: {}", e.what());
        }
    }
    else
    {
        // LogError("Policy file does not contain policy");
        return nullptr;
    }

    if (const auto requirementsNode = m_node["requirements"]; requirementsNode)
    {
        try
        {
            requirements.condition = requirementsNode["condition"].as<std::string>();
            ValidateConditionString(requirements.condition);

            for (const auto& rule : requirementsNode["rules"])
            {
                std::unique_ptr<IRuleEvaluator> RuleEvaluator =
                    RuleEvaluatorFactory::CreateEvaluator(rule.as<std::string>());
                if (RuleEvaluator != nullptr)
                {
                    requirements.rules.push_back(std::move(RuleEvaluator));
                }
                else
                {
                    // LogError("Failed to parse rule: {}", rule.as<std::string>());
                }
            }
            // LogDebug("Requirements parsed.");
        }
        catch (const std::exception& e)
        {
            // LogError("Failed to parse requirements. Error: {}", e.what());
            return nullptr;
        }
    }

    if (const auto checksNode = m_node["checks"]; checksNode)
    {
        for (const auto& checkNode : checksNode)
        {
            try
            {
                Check check;
                check.id = checkNode["id"].as<std::string>();
                check.condition = checkNode["condition"].as<std::string>();
                ValidateConditionString(check.condition);
                YAML::Node checkWithValidRules = YAML::Clone(checkNode);
                checkWithValidRules["rules"] = YAML::Node(YAML::NodeType::Sequence);

                if (checkNode["rules"])
                {
                    for (const auto& rule : checkNode["rules"])
                    {
                        const auto ruleStr = rule.as<std::string>();
                        if (auto ruleEvaluator = RuleEvaluatorFactory::CreateEvaluator(ruleStr))
                        {
                            check.rules.push_back(std::move(ruleEvaluator));
                            checkWithValidRules["rules"].push_back(ruleStr);
                        }
                        else
                        {
                            // LogError("Failed to parse rule: {}", rule.as<std::string>());
                        }
                    }
                }

                // LogDebug("Check {} parsed.", check.id.value_or("Invalid id"));
                checks.push_back(std::move(check));
                nlohmann::json checkJson = YamlNodeToJson(checkWithValidRules);
                checkJson["policy_id"] = policyId;
                policiesAndChecks["checks"].push_back(checkJson);
            }
            catch (const std::exception& e)
            {
                // LogError("Failed to parse a check. Skipping it. Error: {}", e.what());
                continue;
            }
        }
    }
    else
    {
        // LogError("Policy file does not contain checks");
        return nullptr;
    }

    return std::make_unique<SCAPolicy>(policyId, std::move(requirements), std::move(checks));
}

// NOLINTNEXTLINE(misc-no-recursion)
void PolicyParser::ReplaceVariablesInNode(YAML::Node& currentNode)
{
    if (currentNode.IsScalar())
    {
        auto value = currentNode.as<std::string>();
        for (const auto& pair : m_variablesMap)
        {
            size_t pos = 0;
            while ((pos = value.find(pair.first, pos)) != std::string::npos)
            {
                value.replace(pos, pair.first.length(), pair.second);
                pos += pair.second.length();
            }
        }
        currentNode = value;
    }
    else if (currentNode.IsMap())
    {
        for (auto it = currentNode.begin(); it != currentNode.end(); ++it)
        {
            ReplaceVariablesInNode(it->second);
        }
    }
    else if (currentNode.IsSequence())
    {
        // NOLINTNEXTLINE(modernize-loop-convert)
        for (std::size_t i = 0; i < currentNode.size(); ++i)
        {
            YAML::Node element = currentNode[i];
            ReplaceVariablesInNode(element);
            currentNode[i] = element;
        }
    }
}
