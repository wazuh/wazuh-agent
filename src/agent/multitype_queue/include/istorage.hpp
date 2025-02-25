#pragma once

#include <nlohmann/json.hpp>

#include <memory>
#include <mutex>
#include <string>

/// @brief Interface for Storage
class IStorage
{
public:
    /// @brief Destructor
    virtual ~IStorage() = default;

    /// @brief Clears all messages from the database
    /// @param tableNames A vector of table names
    /// @return True if successful, false otherwise
    virtual bool Clear(const std::vector<std::string>& tableNames) = 0;

    /// @brief Store a JSON message in the storage.
    /// @param message The JSON message to store.
    /// @param tableName The name of the table to store the message in.
    /// @param moduleName The name of the module that created the message.
    /// @param moduleType The type of the module that created the message.
    /// @param metadata The metadata message to store.
    /// @return The number of stored elements.
    virtual int Store(const nlohmann::json& message,
                      const std::string& tableName,
                      const std::string& moduleName = "",
                      const std::string& moduleType = "",
                      const std::string& metadata = "") = 0;

    /// @brief Remove multiple JSON messages.
    /// @param n The number of messages to remove.
    /// @param tableName The name of the table to remove the message from.
    /// @param moduleName The name of the module that created the message.
    /// @param moduleType The module type that created the message.
    /// @return The number of removed elements.
    virtual int RemoveMultiple(int n,
                               const std::string& tableName,
                               const std::string& moduleName = "",
                               const std::string& moduleType = "") = 0;

    /// @brief Retrieve multiple JSON messages.
    /// @param n The number of messages to retrieve.
    /// @param tableName The name of the table to retrieve the message from.
    /// @param moduleName The name of the module that created the message.
    /// @param moduleType The module type that created the message.
    /// @return A vector of retrieved JSON messages.
    virtual nlohmann::json RetrieveMultiple(int n,
                                            const std::string& tableName,
                                            const std::string& moduleName = "",
                                            const std::string& moduleType = "") = 0;

    /// @brief Retrieve multiple JSON messages based on size from the specified queue.
    /// @param n size occupied by the messages to be retrieved.
    /// @param tableName The name of the table to retrieve the message from.
    /// @param moduleName The name of the module.
    /// @param moduleType The type of the module.
    /// @return nlohmann::json The retrieved JSON messages.
    virtual nlohmann::json RetrieveBySize(size_t n,
                                          const std::string& tableName,
                                          const std::string& moduleName = "",
                                          const std::string& moduleType = "") = 0;

    /// @brief Get the number of elements in the table.
    /// @param tableName The name of the table to retrieve the message from.
    /// @param moduleName The name of the module that created the message.
    /// @param moduleType The module type that created the message.
    /// @return The number of elements in the table.
    virtual int GetElementCount(const std::string& tableName,
                                const std::string& moduleName = "",
                                const std::string& moduleType = "") = 0;

    /// @brief Get the bytes occupied by elements stored in the specified queue.
    /// @param tableName  The name of the table.
    /// @param moduleName The name of the module.
    /// @param moduleType The type of the module.
    /// @return size_t The bytes occupied by elements stored in the specified queue.
    virtual size_t GetElementsStoredSize(const std::string& tableName,
                                         const std::string& moduleName = "",
                                         const std::string& moduleType = "") = 0;
};
