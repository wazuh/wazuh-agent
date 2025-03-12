#pragma once

#include <math.h>
#include <memory>
#include <pal.h>
#include <stdexcept>
#include <string>

/// @brief Forward declaration
struct sqlite3;

namespace SQLiteLegacy
{
    const constexpr auto MAX_ROWS_ERROR_STRING {"Too Many Rows."};

    /// @brief This class should be used by concrete types to report errors
    class sqlite_error : public std::exception
    {
    public:
        /// @brief Returns the error message
        /// @returns the error message
        ATTR_RET_NONNULL
        const char* what() const noexcept override
        {
            return m_error.what();
        }

        /// @brief Returns the error id
        /// @return the error id
        int id() const noexcept
        {
            return m_id;
        }

        /// @brief Constructor
        /// @param id id of the error
        /// @param whatArg message of the error
        sqlite_error(const int id, const std::string& whatArg)
            : m_id {id}
            , m_error {whatArg}
        {
        }

        /// @brief Constructor
        /// @param exceptionInfo error information
        explicit sqlite_error(const std::pair<int, std::string>& exceptionInfo)
            : m_id {exceptionInfo.first}
            , m_error {exceptionInfo.second}
        {
        }

    private:
        /// an exception object as storage for error messages
        const int m_id;
        std::runtime_error m_error;
    };

    /// @brief Interface for connections
    class IConnection
    {
    public:
        /// @brief Virtual destructor
        virtual ~IConnection() = default;

        /// @brief Closes the connection
        virtual void close() = 0;

        /// @brief Executes a query
        /// @param query query
        virtual void execute(const std::string& query) = 0;

        /// @brief Returns the number of changes
        /// @return Number of changes
        virtual int64_t changes() const = 0;

        /// @brief Returns the database pointer
        /// @return Database pointer
        virtual const std::shared_ptr<sqlite3>& db() const = 0;
    };

    /// @brief Interface for transactions
    class ITransaction
    {
    public:
        /// @brief Virtual destructor
        virtual ~ITransaction() = default;

        /// @brief Commits the transaction
        virtual void commit() = 0;

        /// @brief Rollbacks the transaction
        virtual void rollback() = 0;
    };

    /// @brief Interface for columns
    class IColumn
    {
    public:
        /// @brief Virtual destructor
        virtual ~IColumn() = default;

        /// @brief Returns the column type
        /// @return Column type
        virtual int32_t type() const = 0;

        /// @brief Returns the column name
        /// @return Column name
        virtual std::string name() const = 0;

        /// @brief Checks if the column has a value
        /// @return True if the column has a value
        virtual bool hasValue() const = 0;

        /// @brief Returns the column value as int32_t
        /// @return Column value
        virtual int32_t value(const int32_t&) const = 0;

        /// @brief Returns the column value as uint64_t
        /// @return Column value
        virtual uint64_t value(const uint64_t&) const = 0;

        /// @brief Returns the column value as int64_t
        /// @return Column value
        virtual int64_t value(const int64_t&) const = 0;

        /// @brief Returns the column value as string
        /// @return Column value
        virtual std::string value(const std::string&) const = 0;

        /// @brief Returns the column value as double_t
        /// @return Column value
        virtual double_t value(const double_t&) const = 0;
    };

    /// @brief Interface for statements
    class IStatement
    {
    public:
        /// @brief Virtual destructor
        virtual ~IStatement() = default;

        /// @brief Steps the statement
        /// @return Step result
        virtual int32_t step() = 0;

        /// @brief Binds a value
        /// @param index index of the column
        /// @param value value as int32_t
        virtual void bind(const int32_t index, const int32_t value) = 0;

        /// @brief Binds a value
        /// @param index index of the column
        /// @param value value as uint64_t
        virtual void bind(const int32_t index, const uint64_t value) = 0;

        /// @brief Binds a value
        /// @param index index of the column
        /// @param value value as int64_t
        virtual void bind(const int32_t index, const int64_t value) = 0;

        /// @brief Binds a value
        /// @param index index of the column
        /// @param value value as string
        virtual void bind(const int32_t index, const std::string& value) = 0;

        /// @brief Binds a value
        /// @param index index of the column
        /// @param value value as double_t
        virtual void bind(const int32_t index, const double_t value) = 0;

        /// @brief Binds null
        /// @param index index of the column
        virtual void bind(const int32_t index) = 0;

        /// @brief Returns the number of columns
        /// @return Number of columns
        virtual int columnsCount() const = 0;

        /// @brief Expands the statement
        /// @return Expanded statement
        virtual std::string expand() = 0;

        /// @brief Returns a column
        /// @param index Column index
        /// @return Column
        virtual std::unique_ptr<IColumn> column(const int32_t index) = 0;

        /// @brief Resets the statement
        virtual void reset() = 0;
    };
} // namespace SQLiteLegacy
