#include <configuration_parser.hpp>

#include <algorithm>
#include <cctype>
#include <config.h>

#include <queue>
#include <unordered_set>
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
    ConfigurationParser::ConfigurationParser(std::filesystem::path configFilePath)
        : m_config_file_path(std::move(configFilePath))
    {
        LoadLocalConfig();
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

    void ConfigurationParser::LoadLocalConfig()
    {
        LogDebug("Loading local config file: {}.", m_config_file_path.string());

        try
        {
            if (!isValidYamlFile(m_config_file_path))
            {
                throw std::runtime_error("The file does not contain a valid YAML structure.");
            }
            m_config = YAML::LoadFile(m_config_file_path.string());
        }
        catch (const std::exception& e)
        {
            m_config = YAML::Node();
            LogWarn("Using default values due to error parsing wazuh-agent.yml file: {}", e.what());
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
                        // Merge sequences while preserving the order.
                        YAML::Node mergedSequence = YAML::Node(YAML::NodeType::Sequence);

                        // Collect elements from 'override' sequence to preserve insertion order.
                        std::vector<std::pair<std::string, YAML::Node>> overrideElements;
                        for (const YAML::Node& elem : value)
                        {
                            if (elem.IsScalar())
                            {
                                overrideElements.emplace_back(elem.as<std::string>(), elem);
                            }
                            else if (elem.IsMap() && elem.begin() != elem.end())
                            {
                                overrideElements.emplace_back(elem.begin()->first.as<std::string>(), elem);
                            }
                        }

                        // Track which keys from 'override' sequence are merged.
                        std::unordered_set<std::string> mergedKeys;

                        for (const YAML::Node& elem : baseNode[key])
                        {
                            std::string elemKey;

                            // Extract the key based on the type of element.
                            if (elem.IsScalar())
                            {
                                elemKey = elem.as<std::string>();
                            }
                            else if (elem.IsMap() && elem.begin() != elem.end())
                            {
                                elemKey = elem.begin()->first.as<std::string>();
                            }
                            else
                            {
                                // Skip elements that don't fit the expected types.
                                mergedSequence.push_back(elem);
                                continue;
                            }

                            // Common logic for merging elements.
                            auto overrideItem =
                                std::find_if(overrideElements.begin(),
                                             overrideElements.end(),
                                             [&elemKey](const auto& pair) { return pair.first == elemKey; });
                            if (overrideItem != overrideElements.end())
                            {
                                mergedSequence.push_back(overrideItem->second);
                                mergedKeys.insert(overrideItem->first);
                            }
                            else
                            {
                                mergedSequence.push_back(elem);
                            }
                        }

                        // Add remaining elements from 'override' sequence in order.
                        for (const auto& [itemKey, itemNode] : overrideElements)
                        {
                            if (mergedKeys.find(itemKey) == mergedKeys.end())
                            {
                                mergedSequence.push_back(itemNode);
                            }
                        }

                        baseNode[key] = mergedSequence;
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
        LogDebug("Loading shared configuration.");
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

                LogDebug("Loading group configuration file: {}.", groupFile.string());

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
        }
    }

    void ConfigurationParser::SetGetGroupIdsFunction(std::function<std::vector<std::string>()> getGroupIdsFunction)
    {
        m_getGroups = std::move(getGroupIdsFunction);
        LoadSharedConfig();
    }

    void ConfigurationParser::ReloadConfiguration()
    {
        LogInfo("Reload configuration.");

        // Reset saved configuration
        m_config = YAML::Node();

        // Load local configuration
        LoadLocalConfig();

        // Load shared configuration
        LoadSharedConfig();

        LogInfo("Reload configuration done.");
    }

} // namespace configuration
