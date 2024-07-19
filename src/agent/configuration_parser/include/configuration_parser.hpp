#pragma once

#include <toml.hpp>

#include <exception>
#include <iostream>

namespace configuration
{
    class ConfigurationParser
    {
    private:
        std::string configFileName = "wazuh.conf";
        toml::value tbl;

    public:
        ConfigurationParser();
        ConfigurationParser(std::string stringToParse);
        ~ConfigurationParser() {};

        template<typename T, typename... Ks>
        auto GetConfig(Ks... ks)
        {
            try
            {
                auto config = toml::find<T>(tbl, ks...);
                return config;
            }
            catch (const std::exception& e)
            {
                std::cout << "The requested value could not be obtained: " << e.what() << std::endl;
                throw;
            }
        };
    };
} // namespace configuration
