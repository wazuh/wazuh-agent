#pragma once

// clang-format off
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <winreg.h>
#include <cstdio>
#include <functional>
#include <vector>
// clang-format on

namespace Utils
{
    /// @brief Registry class
    class Registry final
    {
    public:
        /// @brief Constructor
        /// @param key key of the registry
        /// @param subKey subkey of the registry
        /// @param access access to the registry
        Registry(const HKEY key, const std::string& subKey = "", const REGSAM access = KEY_READ)
            : m_registryKey {openRegistry(key, subKey, access)}
        {
        }

        /// @brief Destructor
        ~Registry();

        /// @brief Close the registry
        void close();

        /// @brief Get a DWORD value
        /// @param valueName Name of the value
        /// @return Value
        DWORD dword(const std::string& valueName) const;

        /// @brief Get a DWORD value and put it in a reference
        /// @param valueName Name of the value
        /// @param value Reference to the value
        /// @return True if the value was found
        bool dword(const std::string& valueName, DWORD& value) const;

        /// @brief Enumerate the keys of the registry
        /// @return List of keys
        std::vector<std::string> enumerate() const;

        /// @brief Enumerate the values of the registry
        /// @return List of values
        std::vector<std::string> enumerateValueKey() const;

        /// @brief Enumerate the keys of the registry and call the callback for each key
        /// @param callback Callback function
        void enumerate(const std::function<void(const std::string&)>& callback) const;

        /// @brief Enumerate the keys of the registry and store them in a vector
        /// @param values Vector to store the keys
        /// @return True if the keys were found
        bool enumerate(std::vector<std::string>& values) const;

        /// @brief Enumerate the values of the registry and store them in a vector
        /// @param values Vector to store the values
        /// @return True if the values were found
        bool enumerateValueKey(std::vector<std::string>& values) const;

        /// @brief Get the last modification date of the registry
        /// @return Modification date
        std::string keyModificationDate() const;

        /// @brief Get a string value
        /// @param valueName Name of the value
        /// @return Value as a string
        std::string string(const std::string& valueName) const;

        /// @brief Get a QWORD value
        /// @param valueName Name of the value
        /// @param value Reference to store the value
        /// @return True if the value was found
        bool qword(const std::string& valueName, ULONGLONG& value) const;

        /// @brief Get a string value
        /// @param valueName Name of the value
        /// @param value Reference to store the value
        /// @return True if the value was found
        bool string(const std::string& valueName, std::string& value) const;

    private:
        /// @brief Open the registry
        /// @param key Key of the registry
        /// @param subKey Subkey of the registry
        /// @param access Access to the registry
        /// @return Handle to the registry
        HKEY openRegistry(const HKEY key, const std::string& subKey, const REGSAM access);

        HKEY m_registryKey;
    };

    /// @brief Expand the registry path
    /// @param key Key of the registry
    /// @param subKey Subkey of the registry
    /// @param registryKeys Vector to store the keys
    void expandRegistryPath(const HKEY key, const std::string& subKey, std::vector<std::string>& registryKeys);
} // namespace Utils
