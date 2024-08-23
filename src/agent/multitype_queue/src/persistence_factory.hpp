#pragma once

#include <ipersistence.hpp>

#include <any>
#include <memory>
#include <vector>

/**
 * @brief Class to create a persistence.
 *
 */
class PersistenceFactory
{
public:
    /**
     * @brief Types of persistence enum
     *
     */
    enum class PersistenceType
    {
        SQLITE3
    };

    /**
     * @brief Create a persistence
     */
    static std::unique_ptr<IPersistence> createPersistence(PersistenceType type, const std::string& dbName);
};
