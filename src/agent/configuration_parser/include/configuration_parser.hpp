#pragma once

#include <configuration_parser_utils.hpp>
#include <logger.hpp>

#include <yaml-cpp/yaml.h>

#include <ctime>
#include <exception>
#include <filesystem>
#include <limits>
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

        /// @brief Retrieves a configuration value or returns a default value if the key is not found.
        /// This function attempts to retrieve a value from the configuration using the specified keys.
        /// If the value is not found or cannot be converted to the requested type, the provided default
        /// value is returned instead.
        /// @tparam T The expected type of the configuration value.
        /// @tparam Keys The types of the keys used to access the configuration node.
        /// @param defaultValue The value to return if the requested configuration value is not found.
        /// @param keys The sequence of keys to navigate through the configuration hierarchy.
        /// @return The retrieved configuration value if found and valid; otherwise, the default value.
        template<typename T, typename... Keys>
        T GetConfigOrDefault(const T& defaultValue, Keys... keys) const
        {
            if (auto result = GetConfig<T>(keys...))
            {
                return *result;
            }
            return defaultValue;
        }

        /// @brief Specialization for const char* of \ref GetConfigOrDefault that returns a std::string.
        /// @details This specialization is needed to handle the case where
        /// the default value is a string literal (i.e.: coming from config.h).
        template<typename... Keys>
        std::string GetConfigOrDefault(const char* defaultValue, Keys... keys) const
        {
            return GetConfigOrDefault<std::string>(std::string(defaultValue), keys...);
        }

        /// @brief Retrieves a configuration value or returns a default value if the key is not found or the value is
        /// outside the specified range.
        /// @tparam T The expected type of the configuration value.
        /// @tparam Keys The types of the keys used to access the configuration node.
        /// @param defaultValue The value to return if the requested configuration value is not found or out of range.
        /// @param min The optional minimum acceptable value (inclusive). If not provided, no lower bound is applied.
        /// @param max The optional maximum acceptable value (inclusive). If not provided, no upper bound is applied.
        /// @param keys The sequence of keys to navigate through the configuration hierarchy.
        /// @return The retrieved configuration value if found and within range; otherwise, the default value.
        template<typename T, typename... Keys>
        T GetConfigInRangeOrDefault(const T& defaultValue,
                                    std::optional<T> min = std::nullopt,
                                    std::optional<T> max = std::nullopt,
                                    Keys... keys) const
        {
            if ((min && max) && (*min >= *max))
            {
                LogWarn("Invalid range: min value is greater or equal to max value.");
                return defaultValue;
            }

            if (const auto result = GetConfig<T>(keys...))
            {
                if ((!min || *result >= *min) && (!max || *result <= *max))
                {
                    return *result;
                }
            }

            LogWarn("Requested setting is not found or out of range, default value used.");
            return defaultValue;
        }

        /// @brief Fetches a configuration value as in GetConfigInRangeOrDefault, but parses the string value as a
        /// time_t.
        /// @tparam Keys The types of the keys used to access the configuration hierarchy.
        /// @param defaultValue The default value (in string format) to return if the key is not found or the value is
        /// out of range.
        /// @param min The minimum acceptable value (inclusive) as a time_t.
        /// @param max The maximum acceptable value (inclusive) as a time_t.
        /// @param keys The sequence of keys used to navigate through the configuration hierarchy.
        /// @return The parsed configuration value as a time_t, or the default value if not found or out of range.
        template<typename... Keys>
        std::time_t GetTimeConfigInRangeOrDefault(const std::string& defaultValue,
                                                  std::time_t min,
                                                  std::time_t max,
                                                  Keys... keys) const
        {
            return GetParsedConfigInRangeOrDefault(defaultValue, min, max, ParseTimeUnit, keys...);
        }

        /// @brief Fetches a configuration value as in GetConfigOrDefault, but parses the string value as a time_t.
        /// @tparam ...Keys The types of the keys used to access the configuration hierarchy.
        /// @param defaultValue The default value (in string format) to return if the key is not found or the value is
        /// invalid (non positive).
        /// @param ...keys The sequence of keys used to navigate through the configuration hierarchy.
        /// @return The parsed configuration value as a time_t, or the default value if not found or invalid.
        template<typename... Keys>
        std::time_t GetTimeConfigOrDefault(const std::string& defaultValue, Keys... keys) const
        {
            return GetTimeConfigInRangeOrDefault(defaultValue, 1, std::numeric_limits<time_t>::max(), keys...);
        }

        /// @brief Fetches a configuration value as in GetConfigInRangeOrDefault, but parses the string value as a
        /// value in bytes, as in \ref ParseSizeUnit.
        /// @tparam Keys The types of the keys used to access the configuration hierarchy.
        /// @param defaultValue The default value (in string format) to return if the key is not found or the value
        /// is out of range.
        /// @param min The minimum acceptable value (inclusive) as a bytes.
        /// @param max The maximum acceptable value (inclusive) as a bytes.
        /// @param keys The sequence of keys used to navigate through the configuration hierarchy.
        /// @return The parsed configuration value as a bytes, or the default value if not found or out of range.
        template<typename... Keys>
        std::size_t GetBytesConfigInRangeOrDefault(const std::string& defaultValue,
                                                   std::size_t min,
                                                   std::size_t max,
                                                   Keys... keys) const
        {
            return GetParsedConfigInRangeOrDefault(defaultValue, min, max, ParseSizeUnit, keys...);
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
        ///
        /// This function sets the function to get group IDs, after setting the function the shared configuration will
        /// be reloaded using this function.
        ///
        /// @param getGroupIdsFunction A function to get group IDs.
        void SetGetGroupIdsFunction(std::function<std::vector<std::string>()> getGroupIdsFunction);

        /// @brief Method for loading the new available configuration
        void ReloadConfiguration();

    private:
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
                // clang-format off
                ([&current] (const auto& key)
                {
                    current = current[key];

                    if (!current.IsDefined())
                    {
                        throw YAML::Exception(YAML::Mark::null_mark(), "Key not found: " + std::string(key));
                    }
                }(keys), ...);
                // clang-format on

                return current.as<T>();
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

        /// @brief Parses a string configuration value into a type and validates it within a range.
        /// @tparam T The type to parse the configuration value into.
        /// @tparam ParseFunc The function to parse the configuration value into type T.
        /// @tparam Keys The types of the keys used to access the configuration hierarchy.
        /// @param defaultValue The default value to return if the key is not found or the value is out of range.
        /// @param min The minimum acceptable value (inclusive) for the parsed value.
        /// @param max The maximum acceptable value (inclusive) for the parsed value.
        /// @param parseFunc The function to parse the configuration value into type T.
        /// @param keys The sequence of keys used to navigate through the configuration hierarchy.
        /// @return The parsed configuration value if found and within range; otherwise, the default value.
        template<typename T, typename ParseFunc, typename... Keys>
        T GetParsedConfigInRangeOrDefault(
            const std::string& defaultValue, T min, T max, ParseFunc parseFunc, Keys... keys) const
        {
            if (min >= max)
            {
                LogWarn("Invalid range: min value is greater or equal to max value.");
                return parseFunc(defaultValue);
            }

            if (const auto result = GetConfig<std::string>(keys...))
            {
                try
                {
                    const auto parsedResult = parseFunc(*result);

                    if (min <= parsedResult && parsedResult <= max)
                    {
                        return parsedResult;
                    }
                }
                catch (const std::exception& e)
                {
                    LogWarn("Exception while parsing configuration value: {}. Default value used.", e.what());
                }
            }

            LogWarn("Requested setting is not found or out of range, default value used.");
            return parseFunc(defaultValue);
        }

        /// @brief Method for loading the configuration from local file
        void LoadLocalConfig();

        /// @brief Loads shared configuration files for specific groups and merges them into the main configuration.
        ///
        /// This function attempts to load configuration files for each group from a shared directory.
        /// The loaded configurations are merged into the main configuration.
        ///
        /// @throws YAML::Exception If there is an error while loading or parsing a YAML file.
        void LoadSharedConfig();

        /// @brief Holds the parsed YAML configuration.
        YAML::Node m_config;

        /// @brief Holds the location of the configuration file.
        std::filesystem::path m_configFilePath;

        /// @brief Function to get the groups information
        std::function<std::vector<std::string>()> m_getGroups;
    };
} // namespace configuration
