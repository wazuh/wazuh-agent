#pragma once

#include <logger.hpp>

#include <yaml-cpp/yaml.h>

#include <ctime>
#include <exception>
#include <filesystem>
#include <optional>
#include <string>
#include <type_traits>

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

        /// @brief Holds the location of the configuration file.
        std::filesystem::path m_config_file_path;

        /// @brief Converts a time unit represented as a string to an time_t value (ms).
        /// @param option A string representing a time unit.
        /// @return The corresponding time_t value.
        /// @throws std::invalid_argument if the string does not represent a valid time unit.
        /// @details This function parses a string representing a time unit and returns the equivalent time_t
        /// value. The time unit can be expressed in milliseconds (e.g. "1ms"), seconds (e.g. "1s"), minutes (e.g.
        /// "1m"), hours (e.g. "1h"), or days (e.g. "1d"). If no unit is specified, the value is assumed to be in
        /// seconds.
        std::time_t ParseTimeUnit(const std::string& option) const;

        /// @brief The groups information
        std::function<std::vector<std::string>()> m_getGroups;

        /// @brief Merges two YAML nodes, modifying the base node to include or override values from the
        /// override node.
        ///
        /// This function traverses the two YAML nodes. If a key exists in both nodes:
        /// - If both values are maps, the function recurses to merge their content.
        /// - If both values are sequences, their elements are concatenated.
        /// - In all other cases (scalars, aliases, null values), the value from the override node replaces the value in
        /// the base node. If a key only exists in the override node, it is added to the base node.
        ///
        /// @param base Reference to the base YAML::Node that will be modified.
        /// @param override Const reference to the YAML::Node containing values to merge into the base.
        void MergeYamlNodes(YAML::Node& base, const YAML::Node& override);

        /// @brief Method for loading the configuration from local file
        void LoadLocalConfig();

        /// @brief Loads shared configuration files for specific groups and merges them into the main configuration.
        ///
        /// This function attempts to load configuration files for each group from a shared directory.
        /// The loaded configurations are merged into the main configuration.
        ///
        /// @throws YAML::Exception If there is an error while loading or parsing a YAML file.
        void LoadSharedConfig();

    public:
        /// @brief Default constructor. Loads configuration from a default file path.
        ///
        /// Calls the constructor that accepts a file path, using the default configuration file name.
        /// If the file cannot be loaded, it falls back to default configuration values.
        ConfigurationParser();

        /// @brief Constructs a ConfigurationParser and loads the configuration from a specified YAML file.
        /// @param configFilePath The path to the YAML configuration file.
        /// @details This constructor attempts to load configuration data from the specified file path.
        /// If loading fails, it logs an error and falls back to a set of predefined default values.
        ConfigurationParser(std::filesystem::path configFilePath);

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
        /// @return The configuration value corresponding to the specified keys or std::nullopt.
        /// @details This method provides a flexible way to retrieve deeply nested configuration values using
        /// a variadic sequence of keys, which allows specifying paths within the YAML structure.
        template<typename T, typename... Keys>
        std::optional<T> GetConfig(Keys... keys) const
        {
            YAML::Node current = YAML::Clone(m_config);

            try
            {
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

                if constexpr (std::is_same_v<T, std::time_t>)
                {
                    return ParseTimeUnit(current.as<std::string>());
                }
                else
                {
                    return current.as<T>();
                }
            }
            catch (const std::invalid_argument& e)
            {
                LogWarn("Requested setting is invalid, default value used. {}", e.what());
                return std::nullopt;
            }
            catch (const std::exception& e)
            {
                LogDebug("Requested setting not found, default value used. {}", e.what());
                return std::nullopt;
            }
        }

        /// @brief Checks if the specified YAML file is valid.
        ///
        /// This function attempts to load the YAML file located at the given path.
        /// If the file can be loaded without throwing an exception, it is considered valid.
        ///
        /// @param configFile The path to the YAML file to be validated.
        /// @return `true` if the file is a valid YAML file; `false` otherwise.
        bool isValidYamlFile(const std::filesystem::path& configFile) const;

        /// @brief Sets the function to get group IDs.
        /// @param getGroupIdsFunction A function to get group IDs.
        void SetGetGroupIdsFunction(std::function<std::vector<std::string>()> getGroupIdsFunction);

        /// @brief Method for loading the new available configuration
        void ReloadConfiguration();
    };
} // namespace configuration
