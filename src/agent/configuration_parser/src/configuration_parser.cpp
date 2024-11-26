#include <configuration_parser.hpp>

#include <algorithm>
#include <cctype>
#include <config.h>

#include <queue>
#include <utility>

namespace
{
    constexpr unsigned int A_SECOND_IN_MILLIS = 1000;
    constexpr unsigned int A_MINUTE_IN_MILLIS = 60 * A_SECOND_IN_MILLIS;
    constexpr unsigned int A_HOUR_IN_MILLIS = 60 * A_MINUTE_IN_MILLIS;
    constexpr unsigned int A_DAY_IN_MILLIS = 24 * A_HOUR_IN_MILLIS;

    const std::filesystem::path CONFIG_FILE = std::filesystem::path(config::DEFAULT_CONFIG_PATH) / "wazuh-agent.yml";
} // namespace

namespace configuration
{
    ConfigurationParser::ConfigurationParser(const std::filesystem::path& configFilePath)
    {
        try
        {
            m_config = YAML::LoadFile(configFilePath.string());

            if (!m_config.IsMap() && !m_config.IsSequence())
            {
                throw std::runtime_error("The file does not contain a valid YAML structure.");
            }
        }
        catch (const std::exception& e)
        {
            m_config = YAML::Node();
            LogWarn("Using default values due to error parsing wazuh-agent.yml file: {}", e.what());
        }
    }

    ConfigurationParser::ConfigurationParser()
        : ConfigurationParser(CONFIG_FILE)
    {
    }

    ConfigurationParser::ConfigurationParser(const std::string& stringToParse)
    {
        try
        {
            m_config = YAML::Load(stringToParse);
        }
        catch (const std::exception& e)
        {
            LogError("Error parsing yaml string: {}.", e.what());
            throw;
        }
    }

    std::time_t ConfigurationParser::ParseTimeUnit(const std::string& option) const
    {
        std::string number;
        unsigned int multiplier = 1;

        if (option.ends_with("ms"))
        {
            number = option.substr(0, option.length() - 2);
        }
        else if (option.ends_with("s"))
        {
            number = option.substr(0, option.length() - 1);
            multiplier = A_SECOND_IN_MILLIS;
        }
        else if (option.ends_with("m"))
        {
            number = option.substr(0, option.length() - 1);
            multiplier = A_MINUTE_IN_MILLIS;
        }
        else if (option.ends_with("h"))
        {
            number = option.substr(0, option.length() - 1);
            multiplier = A_HOUR_IN_MILLIS;
        }
        else if (option.ends_with("d"))
        {
            number = option.substr(0, option.length() - 1);
            multiplier = A_DAY_IN_MILLIS;
        }
        else
        {
            // By default, assume seconds
            number = option;
            multiplier = A_SECOND_IN_MILLIS;
        }

        if (!std::all_of(number.begin(), number.end(), static_cast<int (*)(int)>(std::isdigit)))
        {
            throw std::invalid_argument("Invalid time unit: " + option);
        }

        return static_cast<std::time_t>(std::stoul(number) * multiplier);
    }

    bool ConfigurationParser::isValidYamlFile(const std::filesystem::path& configFile) const
    {
        try
        {
            YAML::Node mapToValidte = YAML::LoadFile(configFile.string());
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

    void ConfigurationParser::MergeYamlNodes(YAML::Node& base, const YAML::Node& override)
    {
        // Queue to manage nodes to be merged. Pairs of nodes are handled directly.
        std::queue<std::pair<YAML::Node, YAML::Node>> nodesToProcess;
        nodesToProcess.emplace(base, override);

        while (!nodesToProcess.empty())
        {
            auto [baseNode, overrideNode] = nodesToProcess.front();
            nodesToProcess.pop();

            // Traverse each key-value pair in the override node.
            for (auto it = overrideNode.begin(); it != overrideNode.end(); ++it)
            {
                const auto key = it->first.as<std::string>();
                YAML::Node value = it->second;

                if (baseNode[key])
                {
                    // Key exists in the base node.
                    if (value.IsMap() && baseNode[key].IsMap())
                    {
                        // Both values are maps: enqueue for further merging.
                        nodesToProcess.emplace(baseNode[key], value);
                    }
                    else if (value.IsSequence() && baseNode[key].IsSequence())
                    {
                        // Both values are sequences(lists): concatenate the elements.
                        for (const auto& elem : value)
                        {
                            baseNode[key].push_back(elem);
                        }
                    }
                    else
                    {
                        // Other cases (scalar, alias, null): overwrite the value.
                        baseNode[key] = value;
                    }
                }
                else
                {
                    // Key does not exist in the base node: add it directly.
                    baseNode[key] = value;
                }
            }
        }
    }

    void ConfigurationParser::LoadSharedConfig()
    {
        if (m_getGroups == nullptr)
        {
            LogWarn("Load shared configuration failed, no get groups function set");
            return;
        }

        try
        {
            const std::vector<std::string> groupIds = m_getGroups();
            YAML::Node tmpConfig = m_config;

            for (const auto& groupId : groupIds)
            {
                const std::filesystem::path groupFile =
                    std::filesystem::path(config::DEFAULT_SHARED_CONFIG_PATH) / (groupId + ".conf");

                YAML::Node fileToAppend = YAML::LoadFile(groupFile.string());

                if (!tmpConfig.IsDefined() || tmpConfig.IsNull())
                {
                    tmpConfig = fileToAppend;
                }
                else
                {
                    MergeYamlNodes(tmpConfig, fileToAppend);
                }
            }

            m_config = tmpConfig;
        }
        catch (const YAML::Exception& e)
        {
            LogWarn("Load shared configuration failed: {}", e.what());
            throw;
        }
    }

    void ConfigurationParser::SetGetGroupIdsFunction(std::function<std::vector<std::string>()> getGroupIdsFunction)
    {
        m_getGroups = std::move(getGroupIdsFunction);
        LoadSharedConfig();
    }

} // namespace configuration
