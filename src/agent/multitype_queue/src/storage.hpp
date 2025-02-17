#pragma once

#include <persistence.hpp>

#include <nlohmann/json.hpp>

#include <memory>
#include <mutex>
#include <string>

class Persistence;

/// @brief Storage class.
///
/// This class provides methods to store, retrieve, and remove JSON messages
/// in a database.
class Storage
{
public:
    /// @brief Constructor
    /// @param dbFolderPath The path to the database folder
    /// @param tableNames A vector of table names
    /// @param persistence Optional pointer to an existing persistence object.
    Storage(const std::string& dbFolderPath,
            const std::vector<std::string>& tableNames,
            std::unique_ptr<Persistence> persistence = nullptr);

    /// @brief Delete copy constructor
    Storage(const Storage&) = delete;

    /// @brief Delete copy assignment operator
    Storage& operator=(const Storage&) = delete;

    /// @brief Delete move constructor
    Storage(Storage&&) = delete;

    /// @brief Delete move assignment operator
    Storage& operator=(Storage&&) = delete;

    /// @brief Destructor
    ~Storage();

    /// @brief Clears all messages from the database
    /// @param tableNames A vector of table names
    /// @return True if successful, false otherwise
    bool Clear(const std::vector<std::string>& tableNames);

    /// @brief Store a JSON message in the storage.
    /// @param message The JSON message to store.
    /// @param tableName The name of the table to store the message in.
    /// @param moduleName The name of the module that created the message.
    /// @param moduleType The type of the module that created the message.
    /// @param metadata The metadata message to store.
    /// @return The number of stored elements.
    int Store(const nlohmann::json& message,
              const std::string& tableName,
              const std::string& moduleName = "",
              const std::string& moduleType = "",
              const std::string& metadata = "");

    /// @brief Remove multiple JSON messages.
    /// @param n The number of messages to remove.
    /// @param tableName The name of the table to remove the message from.
    /// @param moduleName The name of the module that created the message.
    /// @param moduleType The module type that created the message.
    /// @return The number of removed elements.
    int RemoveMultiple(int n,
                       const std::string& tableName,
                       const std::string& moduleName = "",
                       const std::string& moduleType = "");

    /// @brief Retrieve multiple JSON messages.
    /// @param n The number of messages to retrieve.
    /// @param tableName The name of the table to retrieve the message from.
    /// @param moduleName The name of the module that created the message.
    /// @param moduleType The module type that created the message.
    /// @return A vector of retrieved JSON messages.
    nlohmann::json RetrieveMultiple(int n,
                                    const std::string& tableName,
                                    const std::string& moduleName = "",
                                    const std::string& moduleType = "");

    /// @brief Retrieve multiple JSON messages based on size from the specified queue.
    /// @param n size occupied by the messages to be retrieved.
    /// @param tableName The name of the table to retrieve the message from.
    /// @param moduleName The name of the module.
    /// @param moduleType The type of the module.
    /// @return nlohmann::json The retrieved JSON messages.
    nlohmann::json RetrieveBySize(size_t n,
                                  const std::string& tableName,
                                  const std::string& moduleName = "",
                                  const std::string& moduleType = "");

    /// @brief Get the number of elements in the table.
    /// @param tableName The name of the table to retrieve the message from.
    /// @param moduleName The name of the module that created the message.
    /// @param moduleType The module type that created the message.
    /// @return The number of elements in the table.
    int GetElementCount(const std::string& tableName,
                        const std::string& moduleName = "",
                        const std::string& moduleType = "");

    /// @brief Get the bytes occupied by elements stored in the specified queue.
    /// @param tableName  The name of the table.
    /// @param moduleName The name of the module.
    /// @param moduleType The type of the module.
    /// @return size_t The bytes occupied by elements stored in the specified queue.
    size_t GetElementsStoredSize(const std::string& tableName,
                                 const std::string& moduleName = "",
                                 const std::string& moduleType = "");

private:
    /// @brief Create a table in the database.
    /// @param tableName The name of the table to create.
    void CreateTable(const std::string& tableName);

    /// @brief Pointer to the database connection.
    std::unique_ptr<Persistence> m_db;

    /// @brief Mutex to ensure thread-safe operations.
    std::mutex m_mutex;
};
