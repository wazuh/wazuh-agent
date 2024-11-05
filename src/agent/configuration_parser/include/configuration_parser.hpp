#pragma once

#include <logger.hpp>

#include <yaml-cpp/yaml.h>

#include <exception>
#include <filesystem>
#include <string>

namespace configuration
{
    /// @class ConfigurationParser
    /// @brief A parser for loading and retrieving configuration values from YAML files or strings.
    ///
    /// This class allows configuration data to be loaded from a specified file or directly from a YAML string,
    /// and provides methods to access configuration parameters.
    class ConfigurationParser
    {
    private:
        /// @brief Holds the parsed YAML configuration.
        YAML::Node m_config;

    public:
        /// @brief Default constructor. Loads configuration from a default file path.
        ///
        /// Calls the constructor that accepts a file path, using the default configuration file name.
        /// If the file cannot be loaded, it falls back to default configuration values.
        ConfigurationParser();

        /// @brief Constructs a ConfigurationParser and loads the configuration from a specified YAML file.
        /// @param configFile The path to the YAML configuration file.
        /// @details This constructor attempts to load configuration data from the specified file path.
        /// If loading fails, it logs an error and falls back to a set of predefined default values.
        ConfigurationParser(const std::filesystem::path& configFile);

        /// @brief Constructs a ConfigurationParser from a YAML-formatted string.
        /// @param stringToParse A string containing YAML data to parse.
        /// @throws std::exception if parsing the YAML string fails.
        /// @details This constructor allows YAML configuration to be loaded directly from a string.
        /// If parsing fails, an error is logged, and the exception is re-thrown.
        ConfigurationParser(const std::string& stringToParse);

        /// @brief Retrieves a configuration value by following a sequence of nested keys.
        /// @tparam T The expected type of the configuration value to retrieve.
        /// @tparam Keys Variadic template parameters representing the hierarchical path to the desired value.
        /// @param keys A sequence of keys to locate the configuration value within the YAML structure.
        /// @return The configuration value corresponding to the specified keys.
        /// @throws std::exception if the specified path is invalid or if the type does not match the expected type `T`.
        /// @details This method provides a flexible way to retrieve deeply nested configuration values using
        /// a variadic sequence of keys, which allows specifying paths within the YAML structure.
        template<typename T, typename... Keys>
        T GetConfig(Keys... keys) const
        {
            try
            {
                YAML::Node current = YAML::Clone(m_config);

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
