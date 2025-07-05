// main.cpp
#include "../src/libyaml_wrapper.hpp"
#include <filesystem>
#include <iostream>

struct StringLengthGreater
{
    bool operator()(const std::string& a, const std::string& b) const
    {
        return a.length() > b.length() || (a.length() == b.length() && a < b);
    }
};

std::map<std::string, std::string, StringLengthGreater> m_variablesMap;

// NOLINTNEXTLINE(misc-no-recursion)
void ReplaceVariablesInNode(YamlNode& currentNode)
{
    if (currentNode.IsScalar())
    {
        auto value = currentNode.AsString();
        for (const auto& pair : m_variablesMap)
        {
            size_t pos = 0;
            while ((pos = value.find(pair.first, pos)) != std::string::npos)
            {
                value.replace(pos, pair.first.length(), pair.second);
                pos += pair.second.length();
            }
        }
        currentNode.SetScalarValue(value);
    }
    else if (currentNode.IsMap())
    {
        for (auto& [key, node] : currentNode.AsMap())
        {
            ReplaceVariablesInNode(node);
        }
    }
    else if (currentNode.IsSequence())
    {
        auto items = currentNode.AsSequence();
        for (auto& item : items)
        {
            ReplaceVariablesInNode(item);
        }
    }
}

int main()
{
    try
    {
        // Load YAML from file
        YamlDocument doc(std::filesystem::path("sample.yaml"));
        YamlNode root = doc.GetRoot();

        // std::cout << "------------------------\n";
        // root.DumpYamlStructure();
        // std::cout << "------------------------\n";

        // Access a scalar value: policy.id
        std::string policy_id = root["policy"]["id"].AsString();
        std::cout << "Policy ID: " << policy_id << std::endl;

        // Modify the value
        root["policy"]["id"].SetScalarValue("modified_id");
        std::cout << "Modified Policy ID: " << root["policy"]["id"].AsString() << std::endl;

        // Access a sequence: requirements.rules
        std::cout << "\nRules under requirements:" << std::endl;
        auto rules = root["requirements"]["rules"].AsSequence();
        for (const auto& rule : rules)
        {
            std::cout << "- " << rule.AsString() << std::endl;
        }

        // Access a map in checks
        auto checks = root["checks"].AsSequence();
        for (const auto& check : checks)
        {
            std::cout << "\nCheck ID: " << check["id"].AsString() << std::endl;
            std::cout << "Title: " << check["title"].AsString() << std::endl;

            auto rules = check["rules"].AsSequence();
            for (const auto& rule : rules)
            {
                std::cout << "- " << rule.AsString() << std::endl;
            }
        }

        if (root.HasKey("variables"))
        {
            auto variablesNode = root["variables"];

            std::cout << "Variables:\n";
            for (const auto& [key, val] : variablesNode.AsMap())
            {
                const auto name = key;
                const auto value = val.AsString();
                std::cout << "Name: " << name << ", Value: " << value << "\n";
            }
        }

        std::cout << "*********************************\n";
        // collect the variables
        if (root.HasKey("variables"))
        {
            auto variablesNode = root["variables"];

            for (const auto& [key, val] : variablesNode.AsMap())
            {
                m_variablesMap[key] = val.AsString();
            }

            ReplaceVariablesInNode(root);
        }

        // Access a sequence: requirements.rules
        std::cout << "\nModified rules under requirements:" << std::endl;
        auto modRules = root["requirements"]["rules"].AsSequence();
        for (const auto& rule : modRules)
        {
            std::cout << "- " << rule.AsString() << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
