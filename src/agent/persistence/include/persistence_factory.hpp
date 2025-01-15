#pragma once

#include <persistence.hpp>

#include <any>
#include <memory>
#include <vector>

/// @brief Class to create a persistence.
class PersistenceFactory
{
public:
    /// @brief Types of persistence enum.
    enum class PersistenceType
    {
        SQLITE3
    };

    /// @brief Create a persistence.
    /// @param type Type of persistence.
    /// @param dbName Name of the database.
    /// @return A unique pointer to the created persistence.
    static std::unique_ptr<Persistence> CreatePersistence(PersistenceType type, const std::string& dbName);
};
