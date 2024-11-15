#include <configuration_parser.hpp>

#include <utility>

namespace
{
    constexpr unsigned int A_SECOND_IN_MILLIS = 1000;
    constexpr unsigned int A_MINUTE_IN_MILLIS = 60 * A_SECOND_IN_MILLIS;
    constexpr unsigned int A_HOUR_IN_MILLIS = 60 * A_MINUTE_IN_MILLIS;
    constexpr unsigned int A_DAY_IN_MILLIS = 24 * A_HOUR_IN_MILLIS;

#ifdef _WIN32
    /// @brief Gets the path to the configuration file.
    ///
    /// On Windows, this method queries the environment variable ProgramData and
    /// constructs the path to the configuration file as
    /// %ProgramData%\\wazuh-agent\\config\\wazuh-agent.yml. If the environment variable
    /// is not set, it falls back to the default path
    /// C:\\ProgramData\\wazuh-agent\\config\\wazuh-agent.yml.
    ///
    /// @return The path to the configuration file.
    std::string getConfigFilePath()
    {
        std::string configFilePath;
        char* programData = nullptr;
        std::size_t len = 0;
        int error = _dupenv_s(&programData, &len, "ProgramData");

        if (error || programData == nullptr)
        {
            configFilePath = "C:\\ProgramData\\wazuh-agent\\config\\wazuh-agent.yml";
        }
        else
        {
            configFilePath = std::string(programData) + "\\wazuh-agent\\config\\wazuh-agent.yml";
        }
        free(programData);
        return configFilePath;
    }

    const std::string CONFIG_FILE_NAME = getConfigFilePath();
#else
    const std::string CONFIG_FILE_NAME = "/etc/wazuh-agent/wazuh-agent.yml";
#endif
} // namespace

namespace configuration
{
    ConfigurationParser::ConfigurationParser(const std::filesystem::path& configFile)
    {
        try
        {
            m_config = YAML::LoadFile(configFile.string());
        }
        catch (const std::exception& e)
        {
            LogWarn("Using default values due to error parsing wazuh-agent.yml file: {}", e.what());
        }
    }

    ConfigurationParser::ConfigurationParser()
        : ConfigurationParser(std::filesystem::path(CONFIG_FILE_NAME))
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

    unsigned long ConfigurationParser::ParseTimeUnit(const std::string& option)
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

        if (all_of(number.begin(), number.end(), ::isdigit) == false)
        {
            throw std::invalid_argument("Invalid time unit: " + option);
        }

        return static_cast<unsigned long>(std::stoul(number) * multiplier);
    }
} // namespace configuration
