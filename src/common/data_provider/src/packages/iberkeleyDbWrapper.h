#pragma once

#include "db.h"
#include <cstring>
#include <memory>

/// @brief Interface for Berkeley DB
class IBerkeleyDbWrapper
{
public:
    /// @brief Gets a row from the database
    /// @param key Key of the row
    /// @param data Data of the row
    /// @return Row number
    virtual int32_t getRow(DBT& key, DBT& data) = 0;

    /// @brief Default destructor
    virtual ~IBerkeleyDbWrapper() = default;

    /// @brief Default constructor
    IBerkeleyDbWrapper() = default;
};
