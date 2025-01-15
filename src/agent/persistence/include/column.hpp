#pragma once

#include <string>
#include <vector>

namespace column
{
    /// @brief Logical operators for combining selection criteria in queries.
    enum class LogicalOperator
    {
        AND,
        OR
    };

    /// @brief Supported order types for sorting results.
    enum class OrderType
    {
        ASC,
        DESC
    };

    /// @brief Supported column data types for tables.
    enum class ColumnType
    {
        INTEGER,
        TEXT,
        REAL
    };

    /// @brief Supported column attributes for tables.
    enum
    {
        NONE = 0,
        NOT_NULL = 1 << 0,
        PRIMARY_KEY = 1 << 1,
        AUTO_INCREMENT = 1 << 2
    };

    /// @brief Represents a database column.
    class ColumnName
    {
    public:
        /// @brief Constructor for defining a table column.
        /// @param name The name of the column.
        /// @param type The data type of the column (INTEGER, TEXT, or REAL).
        ColumnName(std::string name, const ColumnType type)
            : Name(std::move(name))
            , Type(type)
        {
        }

        /// @brief The name of the column
        std::string Name;

        /// @brief The type of the column
        ColumnType Type;
    };

    /// @brief Represents a database column with attributes.
    class ColumnKey : public ColumnName
    {
    public:
        /// @brief Constructor for defining a table column with attributes.
        /// @param name The name of the column.
        /// @param type The data type of the column (INTEGER, TEXT, or REAL).
        /// @param attributes The attributes of the column (NOT_NULL, PRIMARY_KEY, AUTO_INCREMENT).
        ColumnKey(std::string name, const ColumnType type, const int attributes = NONE)
            : ColumnName(std::move(name), type)
            , Attributes(attributes)
        {
        }

        /// @brief The attributes of the column
        int Attributes;
    };

    /// @brief Represents a database column with data.
    class ColumnValue : public ColumnName
    {
    public:
        /// @brief Constructor for defining a column with a specific value.
        /// @param name The name of the column.
        /// @param type The data type of the column.
        /// @param value The value of the column.
        ColumnValue(std::string name, const ColumnType type, std::string value)
            : ColumnName(std::move(name), type)
            , Value(std::move(value))
        {
        }

        /// @brief The value of the column as a string
        std::string Value;
    };

    using Names = std::vector<ColumnName>;
    using Keys = std::vector<ColumnKey>;
    using Row = std::vector<ColumnValue>;
    using Criteria = std::vector<ColumnValue>;
} // namespace column
