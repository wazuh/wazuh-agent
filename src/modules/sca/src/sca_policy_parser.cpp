#include <sca_policy_parser.hpp>

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>

#include <logger.hpp>

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
} // namespace

PolicyParser::PolicyParser(const std::filesystem::path& filename, const LoadFileFunc& loadFile)
{
    try
    {
        if (!isValidYamlFile(filename))
        {
            throw std::runtime_error("The file does not contain a valid YAML structure.");
        }
        m_node = loadFile(filename);
        auto variables = m_node["variables"];
        if (variables)
        {
            for (const auto& var : variables)
            {
                const auto var_name = var.first.as<std::string>();
                const auto var_value = var.second.as<std::string>();
                m_variable_map[var_name] = var_value;
            }
        }
        replaceVariablesInNode(m_node);
    }
    catch (const YAML::Exception& e)
    {
        LogError("Error parsing YAML file: {}", e.what());
    }
}

bool PolicyParser::isValidYamlFile(const std::filesystem::path& filename) const
{
    try
    {
        const YAML::Node mapToValidte = YAML::LoadFile(filename.string());
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

std::optional<SCAPolicy> PolicyParser::ParsePolicy(nlohmann::json& policiesAndChecks) const
{
    std::vector<SCAPolicy::Check> checks;
    SCAPolicy::Check requirements;

    std::string policyId;

    auto policyNode = m_node["policy"];
    if (policyNode)
    {
        try
        {
            policyId = policyNode["id"].as<std::string>();
            policiesAndChecks["policies"].push_back(YamlNodeToJson(policyNode));

            LogDebug("Policy parsed.");
        }
        catch (const YAML::Exception& e)
        {
            LogError("Failed to parse policy. Skipping it. Error: {}", e.what());
        }
    }
    else
    {
        LogError("Policy file does not contain policy");
        return std::nullopt;
    }

    auto requirementsNode = m_node["requirements"];
    if (requirementsNode)
    {
        try
        {
            requirements.title = requirementsNode["title"].as<std::string>();
            requirements.condition = requirementsNode["condition"].as<std::string>();

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
                    LogError("Failed to parse rule: {}", rule.as<std::string>());
                }
            }
            LogDebug("Requirements parsed.");
        }
        catch (const YAML::Exception& e)
        {
            LogError("Failed to parse requirements. Skipping it. Error: {}", e.what());
            return std::nullopt;
        }
    }

    auto checksNode = m_node["checks"];
    if (checksNode)
    {
        for (const auto& checkNode : checksNode)
        {
            try
            {
                SCAPolicy::Check check;
                check.id = checkNode["id"].as<std::string>();
                check.title = checkNode["title"].as<std::string>();
                check.condition = checkNode["condition"].as<std::string>();

                if (checkNode["rules"])
                {
                    for (const auto& rule : checkNode["rules"])
                    {
                        if (auto ruleEvaluator = RuleEvaluatorFactory::CreateEvaluator(rule.as<std::string>()))
                        {
                            check.rules.push_back(std::move(ruleEvaluator));
                        }
                        else
                        {
                            LogError("Failed to parse rule: {}", rule.as<std::string>());
                        }
                    }
                }

                LogDebug("Check {} parsed.", check.id.value_or("Invalid id"));
                checks.push_back(std::move(check));
                nlohmann::json checkJson = YamlNodeToJson(checkNode);
                checkJson["policy_id"] = policyId;
                policiesAndChecks["checks"].push_back(checkJson);
            }
            catch (const YAML::Exception& e)
            {
                LogError("Failed to parse a check. Skipping it. Error: {}", e.what());
                continue;
            }
        }
    }
    else
    {
        LogError("Policy file does not contain checks");
        return std::nullopt;
    }

    return SCAPolicy(std::move(requirements), std::move(checks));
}

// NOLINTNEXTLINE(misc-no-recursion)
void PolicyParser::replaceVariablesInNode(YAML::Node& currentNode)
{
    if (currentNode.IsScalar())
    {
        auto value = currentNode.as<std::string>();
        for (const auto& pair : m_variable_map)
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
            replaceVariablesInNode(it->second);
        }
    }
    else if (currentNode.IsSequence())
    {
        // NOLINTNEXTLINE(modernize-loop-convert)
        for (std::size_t i = 0; i < currentNode.size(); ++i)
        {
            YAML::Node element = currentNode[i];
            replaceVariablesInNode(element);
            currentNode[i] = element;
        }
    }
}
