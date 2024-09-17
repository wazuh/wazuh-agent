#pragma once

#include <logger.hpp>

#include <toml.hpp>

#include <exception>
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
                LogError("The requested value could not be obtained: {}.", e.what());
                throw;
            }
        }
    };
} // namespace configuration
