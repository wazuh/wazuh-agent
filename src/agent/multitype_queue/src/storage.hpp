#pragma once

#include <istorage.hpp>
#include <persistence.hpp>

#include <nlohmann/json.hpp>

#include <memory>
#include <mutex>
#include <string>

/// @brief Storage class.
///
/// This class provides methods to store, retrieve, and remove JSON messages
/// in a database.
class Storage : public IStorage
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

    /// @copydoc IStorage::Clear
    bool Clear(const std::vector<std::string>& tableNames) override;

    /// @copydoc IStorage::Store
    int Store(const nlohmann::json& message,
              const std::string& tableName,
              const std::string& moduleName = "",
              const std::string& moduleType = "",
              const std::string& metadata = "") override;

    /// @copydoc IStorage::RemoveMultiple
    int RemoveMultiple(int n,
                       const std::string& tableName,
                       const std::string& moduleName = "",
                       const std::string& moduleType = "") override;

    /// @copydoc IStorage::RetrieveMultiple
    nlohmann::json RetrieveMultiple(int n,
                                    const std::string& tableName,
                                    const std::string& moduleName = "",
                                    const std::string& moduleType = "") override;

    /// @copydoc IStorage::RetrieveBySize
    nlohmann::json RetrieveBySize(size_t n,
                                  const std::string& tableName,
                                  const std::string& moduleName = "",
                                  const std::string& moduleType = "") override;

    /// @copydoc IStorage::GetElementCount
    int GetElementCount(const std::string& tableName,
                        const std::string& moduleName = "",
                        const std::string& moduleType = "") override;

    /// @copydoc IStorage::GetElementsStoredSize
    size_t GetElementsStoredSize(const std::string& tableName,
                                 const std::string& moduleName = "",
                                 const std::string& moduleType = "") override;

private:
    /// @brief Create a table in the database.
    /// @param tableName The name of the table to create.
    void CreateTable(const std::string& tableName);

    /// @brief Pointer to the database connection.
    std::unique_ptr<Persistence> m_db;

    /// @brief Mutex to ensure thread-safe operations.
    std::mutex m_mutex;
};
