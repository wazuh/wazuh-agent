#include <configuration_parser.hpp>

#include <algorithm>
#include <cctype>
#include <config.h>
#include <configuration_parser_utils.hpp>

#include <queue>
#include <unordered_set>
#include <utility>

namespace
{
    constexpr unsigned int A_SECOND_IN_MILLIS = 1000;
    constexpr unsigned int A_MINUTE_IN_MILLIS = 60 * A_SECOND_IN_MILLIS;
    constexpr unsigned int A_HOUR_IN_MILLIS = 60 * A_MINUTE_IN_MILLIS;
    constexpr unsigned int A_DAY_IN_MILLIS = 24 * A_HOUR_IN_MILLIS;

    constexpr unsigned int A_KB_IN_BYTES = 1000;
    constexpr unsigned int A_MB_IN_BYTES = 1000 * A_KB_IN_BYTES;
    constexpr unsigned int A_GB_IN_BYTES = 1000 * A_MB_IN_BYTES;

    const std::filesystem::path CONFIG_FILE = std::filesystem::path(config::DEFAULT_CONFIG_PATH) / "wazuh-agent.yml";
} // namespace

namespace configuration
{
    ConfigurationParser::ConfigurationParser(std::filesystem::path configFilePath)
        : m_configFilePath(std::move(configFilePath))
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
        LogDebug("Loading local config file: {}.", m_configFilePath.string());

        try
        {
            if (!isValidYamlFile(m_configFilePath))
            {
                throw std::runtime_error("The file does not contain a valid YAML structure.");
            }
            m_config = YAML::LoadFile(m_configFilePath.string());
        }
        catch (const std::exception& e)
        {
            m_config = YAML::Node();
            LogWarn("Using default values due to error parsing wazuh-agent.yml file: {}", e.what());
        }
    }

    std::time_t ConfigurationParser::ParseTimeUnit(const std::string& option)
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
                const std::filesystem::path groupFile = std::filesystem::path(config::DEFAULT_SHARED_CONFIG_PATH) /
                                                        (groupId + config::DEFAULT_SHARED_FILE_EXTENSION);

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

    size_t ConfigurationParser::ParseSizeUnit(const std::string& option)
    {
        std::string number;
        unsigned int multiplier = 1;

        if (option.ends_with("K"))
        {
            number = option.substr(0, option.length() - 1);
            multiplier = A_KB_IN_BYTES;
        }
        else if (option.ends_with("KB"))
        {
            number = option.substr(0, option.length() - 2);
            multiplier = A_KB_IN_BYTES;
        }
        else if (option.ends_with("M"))
        {
            number = option.substr(0, option.length() - 1);
            multiplier = A_MB_IN_BYTES;
        }
        else if (option.ends_with("MB"))
        {
            number = option.substr(0, option.length() - 2);
            multiplier = A_MB_IN_BYTES;
        }
        else if (option.ends_with("G"))
        {
            number = option.substr(0, option.length() - 1);
            multiplier = A_GB_IN_BYTES;
        }
        else if (option.ends_with("GB"))
        {
            number = option.substr(0, option.length() - 2);
            multiplier = A_GB_IN_BYTES;
        }
        else if (option.ends_with("B"))
        {
            number = option.substr(0, option.length() - 1);
        }
        else
        {
            // By default, assume B
            number = option;
        }

        if (!std::all_of(number.begin(), number.end(), static_cast<int (*)(int)>(std::isdigit)))
        {
            throw std::invalid_argument("Invalid size unit: " + option);
        }

        return static_cast<size_t>(std::stoul(number) * multiplier);
    }
} // namespace configuration
