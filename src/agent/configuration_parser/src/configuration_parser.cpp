#include <configuration_parser.hpp>
#include <utility>

namespace
{
    const std::string CONFIG_FILE_NAME = "wazuh.conf";
}

namespace configuration
{
    ConfigurationParser::ConfigurationParser(const std::filesystem::path& configPath)
    {
        try
        {
            tbl = toml::parse(configPath.string(), toml::spec::v(1, 0, 0));
        }
        catch (const std::exception& e)
        {
            LogError("Using default values due to error parsing wazuh.conf file: {}", e.what());

            tbl = toml::parse_str(
                R"(
                [agent]
                server_mgmt_api_port = "55000"
                agent_comms_api_port = "27000"
                manager_ip = "localhost"
                https_enabled = "yes"

                [inventory]
                enabled = true
                interval = 3600
                scan_on_start = true
                hardware = true
                os = true
                network = true
                packages = true
                ports = true
                ports_all = true
                processes = true
                hotfixes = true

                [logcollector]
                enabled = true
                localfiles = [ "/var/log/auth.log" ]
                reload_interval = 60
                file_wait = 500
                )",
                toml::spec::v(1, 0, 0));
        }
    }

    ConfigurationParser::ConfigurationParser()
        : ConfigurationParser(std::filesystem::path(CONFIG_FILE_NAME))
    {
    }

    ConfigurationParser::ConfigurationParser(std::string stringToParse)
    {
        try
        {
            tbl = toml::parse_str(std::move(stringToParse), toml::spec::v(1, 0, 0));
        }
        catch (const std::exception& e)
        {
            LogError("Error parsing wazuh.conf file: {}.", e.what());
            throw;
        }
    }

} // namespace configuration
