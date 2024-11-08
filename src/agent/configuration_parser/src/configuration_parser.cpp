#include <configuration_parser.hpp>

#include <utility>

namespace
{
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
            LogError("Using default values due to error parsing wazuh-agent.yml file: {}", e.what());

            const std::string yamlStr = R"(
                agent:
                    server_url: https://localhost:27000
                    registration_url: https://localhost:55000
                    retry_interval_secs: 30
                inventory:
                    enabled: true
                    interval: 3600
                    scan_on_start: true
                    hardware: true
                    os: true
                    network: true
                    packages: true
                    ports: true
                    ports_all: true
                    processes: true
                    hotfixes: true
                logcollector:
                    enabled: true
                    localfiles:
                    - /var/log/auth.log
                    reload_interval: 60
                    file_wait: 500
                )";

            m_config = YAML::Load(yamlStr);
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

} // namespace configuration
