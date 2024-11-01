#include <configuration_parser.hpp>
#include <utility>

namespace
{
    const std::string CONFIG_FILE_NAME = "/etc/wazuh-agent/wazuh-agent.yml";
}

namespace configuration
{
    ConfigurationParser::ConfigurationParser(const std::filesystem::path& configFile)
    {
        try
        {
            config = YAML::LoadFile(configFile.string());
        }
        catch (const std::exception& e)
        {
            LogError("Using default values due to error parsing wazuh-agent.yml file: {}", e.what());

            const std::string yamlStr = R"(
                agent:
                    server_url: https://localhost:27000
                    registration_url: https://localhost:55000
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

            config = YAML::Load(yamlStr);
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
            config = YAML::Load(stringToParse);
        }
        catch (const std::exception& e)
        {
            LogError("Error parsing yaml string: {}.", e.what());
            throw;
        }
    }

} // namespace configuration
