#include <configuration_parser.hpp>

namespace configuration
{
    ConfigurationParser::ConfigurationParser()
    {
        try
        {
            tbl = toml::parse(configFileName, toml::spec::v(1, 0, 0));
        }
        catch (const std::exception& e)
        {
            std::cout << "Error parsing wazuh.conf file: " << e.what() << std::endl;
            throw;
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
