#include <configuration_parser.hpp>

namespace
{
    const std::string CONFIG_FILE_NAME = "wazuh.conf";
}

namespace configuration
{
    ConfigurationParser::ConfigurationParser()
    {
        try
        {
            tbl = toml::parse(CONFIG_FILE_NAME, toml::spec::v(1, 0, 0));
        }
        catch (const std::exception& e)
        {
            std::cout << "Using localhost:8080 due to error parsing wazuh.conf file: " << e.what() << std::endl;

            tbl = toml::parse_str(
                R"([agent]
                port = "8080"
                manager_ip = "localhost")",
                toml::spec::v(1, 0, 0));
        }
    }

    ConfigurationParser::ConfigurationParser(std::string stringToParse)
    {
        try
        {
            tbl = toml::parse_str(stringToParse, toml::spec::v(1, 0, 0));
        }
        catch (const std::exception& e)
        {
            std::cout << "Error parsing wazuh.conf file: " << e.what() << std::endl;
            throw;
        }
    }

} // namespace configuration
