#pragma once

#include <logger.hpp>

#include <yaml-cpp/yaml.h>

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

namespace configuration
{
    class ConfigurationParser
    {
    private:
        YAML::Node config;

    public:
        ConfigurationParser();
        ConfigurationParser(const std::filesystem::path& configFile);
        ConfigurationParser(const std::string& stringToParse);

        template<typename T, typename... Keys>
        T GetConfig(Keys... keys) const
        {
            try
            {
                YAML::Node current = YAML::Clone(config);

                (
                    [&current](const auto& key)
                    {
                        current = current[key];
                        if (!current.IsDefined())
                        {
                            throw YAML::Exception(YAML::Mark::null_mark(), "Key not found: " + std::string(key));
                        }
                    }(keys),
                    ...);

                return current.as<T>();
            }
            catch (const std::exception& e)
            {
                LogError("The requested value could not be obtained: {}.", e.what());
                throw;
            }
        }
    };
} // namespace configuration
