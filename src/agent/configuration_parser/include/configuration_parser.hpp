#pragma once

#include <toml.hpp>

#include <exception>
#include <iostream>
#include <string>

namespace configuration
{
    class ConfigurationParser
    {
    private:
        toml::value tbl;

    public:
        ConfigurationParser();
        ConfigurationParser(std::string stringToParse);

        template<typename T, typename... Ks>
        auto GetConfig(Ks... ks) const
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
