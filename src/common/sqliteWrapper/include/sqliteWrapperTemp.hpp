#pragma once

#include "customDeleter.hpp"
#include <chrono>
#include <iostream>
#include <math.h>
#include <memory>
#include <pal.h>
#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include <sys/stat.h>

using DBSyncExceptionType = const std::pair<int, std::string>;

DBSyncExceptionType FACTORY_INSTANTATION {std::make_pair(1, "Unspecified type during factory instantiation")};
DBSyncExceptionType INVALID_HANDLE {std::make_pair(2, "Invalid handle value.")};
DBSyncExceptionType INVALID_TRANSACTION {std::make_pair(3, "Invalid transaction value.")};
DBSyncExceptionType SQLITE_CONNECTION_ERROR {std::make_pair(4, "No connection available for executions.")};
DBSyncExceptionType EMPTY_DATABASE_PATH {std::make_pair(5, "Empty database store path.")};
DBSyncExceptionType EMPTY_TABLE_METADATA {std::make_pair(6, "Empty table metadata.")};
DBSyncExceptionType INVALID_PARAMETERS {std::make_pair(7, "Invalid parameters.")};
DBSyncExceptionType DATATYPE_NOT_IMPLEMENTED {std::make_pair(8, "Datatype not implemented.")};
DBSyncExceptionType SQL_STMT_ERROR {std::make_pair(9, "Invalid SQL statement.")};
DBSyncExceptionType INVALID_PK_DATA {std::make_pair(10, "Primary key not found.")};
DBSyncExceptionType INVALID_COLUMN_TYPE {std::make_pair(11, "Invalid column field type.")};
DBSyncExceptionType INVALID_DATA_BIND {std::make_pair(12, "Invalid data to bind.")};
DBSyncExceptionType INVALID_TABLE {std::make_pair(13, "Invalid table.")};
DBSyncExceptionType INVALID_DELETE_INFO {std::make_pair(14, "Invalid information provided for deletion.")};
DBSyncExceptionType BIND_FIELDS_DOES_NOT_MATCH {
    std::make_pair(15, "Invalid information provided for statement creation.")};
DBSyncExceptionType STEP_ERROR_CREATE_STMT {std::make_pair(16, "Error creating table.")};
DBSyncExceptionType STEP_ERROR_ADD_STATUS_FIELD {std::make_pair(17, "Error adding status field.")};
DBSyncExceptionType STEP_ERROR_UPDATE_STATUS_FIELD {std::make_pair(18, "Error updating status field.")};
DBSyncExceptionType STEP_ERROR_DELETE_STATUS_FIELD {std::make_pair(19, "Error deleting status field.")};
DBSyncExceptionType DELETE_OLD_DB_ERROR {std::make_pair(20, "Error deleting old db.")};
DBSyncExceptionType MIN_ROW_LIMIT_BELOW_ZERO {std::make_pair(21, "Invalid row limit, values below 0 not allowed.")};
DBSyncExceptionType ERROR_COUNT_MAX_ROWS {std::make_pair(22, "Count is less than 0.")};

namespace DbSync
{
    /// @brief This class should be used by concrete types to report errors.
    class dbsync_error : public std::exception
    {
    public:
        /// @brief Gets the error message
        /// @return The error message
        ATTR_RET_NONNULL
        const char* what() const noexcept override
        {
            return m_error.what();
        }

        /// @brief Gets the error id
        /// @return The error id
        int id() const noexcept
        {
            return m_id;
        }

        /// @brief Constructor
        /// @param id error id
        /// @param whatArg error message
        dbsync_error(const int id, const std::string& whatArg)
            : m_id {id}
            , m_error {whatArg}
        {
        }

        /// @brief Constructor
        /// @param exceptionInfo pair of error id and error message
        explicit dbsync_error(const std::pair<int, std::string>& exceptionInfo)
            : m_id {exceptionInfo.first}
            , m_error {exceptionInfo.second}
        {
        }

    private:
        /// an exception object as storage for error messages
        const int m_id;
        std::runtime_error m_error;
    };

    /// @brief This class should be used by concrete types to report errors.
    class max_rows_error : public std::exception
    {
    public:
        /// @brief Gets the error message
        /// @return The error message
        ATTR_RET_NONNULL
        const char* what() const noexcept override
        {
            return m_error.what();
        }

        /// @brief Constructor
        /// @param whatArg error message
        explicit max_rows_error(const std::string& whatArg)
            : m_error {whatArg}
        {
        }

    private:
        /// an exception object as storage for error messages
        std::runtime_error m_error;
    };
} // namespace DbSync

constexpr auto DB_DEFAULT_PATH {"temp.db"};
constexpr auto DB_MEMORY {":memory:"};
constexpr auto DB_PERMISSIONS {0640};

namespace SQLiteLegacy
{
    const constexpr auto MAX_ROWS_ERROR_STRING {"Too Many Rows."};

    /// @brief This class should be used by concrete types to report errors.
    class sqlite_error : public DbSync::dbsync_error
    {
    public:
        /// @brief Constructor
        /// @param exceptionInfo pair of error id and error message
        explicit sqlite_error(const std::pair<const int, const std::string>& exceptionInfo)
            : DbSync::dbsync_error {exceptionInfo.first, "sqlite: " + exceptionInfo.second}
        {
        }
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

using namespace SQLiteLegacy;
using ExpandedSQLPtr = std::unique_ptr<char, CustomDeleter<decltype(&sqlite3_free), sqlite3_free>>;

/// @brief Checks the result of a SQLite call
/// @param result result of the SQLite call
/// @param exceptionString error message
static void checkSqliteResult(const int result, const std::string& exceptionString)
{
    if (SQLITE_OK != result)
    {
        throw sqlite_error {std::make_pair(result, exceptionString)};
    }
}

/// @brief Opens a SQLite database
/// @param path database path
/// @param flags database flags
/// @return Database pointer
static sqlite3* openSQLiteDb(const std::string& path, const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)
{
    sqlite3* pDb {nullptr};
    const auto result {sqlite3_open_v2(path.c_str(), &pDb, flags, nullptr)};
    checkSqliteResult(result, "Unspecified type during initialization of SQLite.");
    return pDb;
}

/// @brief Prepares a SQLite statement
/// @param connection connection pointer
/// @param query query string
/// @return Statement pointer
static sqlite3_stmt* prepareSQLiteStatement(std::shared_ptr<IConnection>& connection, const std::string& query)
{
    sqlite3_stmt* pStatement {nullptr};
    const auto result {sqlite3_prepare_v2(connection->db().get(), query.c_str(), -1, &pStatement, nullptr)};
    checkSqliteResult(result, sqlite3_errmsg(connection->db().get()));
    return pStatement;
}

namespace SQLiteLegacy
{
    /// @brief SQLite connection
    class Connection : public IConnection
    {
    public:
        /// @brief Virtual destructor
        ~Connection() = default;

        /// @brief Constructor
        /// @param path database path
        explicit Connection(const std::string& path)
            : m_db {openSQLiteDb(path),
                    [](sqlite3* p)
                    {
                        sqlite3_close_v2(p);
                    }}
        {
#ifndef _WIN32

            if (path.compare(DB_MEMORY) != 0)
            {
                const auto result {chmod(path.c_str(), DB_PERMISSIONS)};

                if (result != 0)
                {
                    throw sqlite_error {std::make_pair(result, "Error changing permissions of SQLite database.")};
                }

                m_db.reset(openSQLiteDb(path, SQLITE_OPEN_READWRITE), [](sqlite3* p) { sqlite3_close_v2(p); });
            }

#endif
        }

        /// @copydoc IConnection::close
        void close()
        {
            m_db.reset();
        }

        /// @copydoc IConnection::db
        const std::shared_ptr<sqlite3>& db() const
        {
            return m_db;
        }

        /// @brief  Constructor
        Connection()
            : Connection(DB_DEFAULT_PATH)
        {
        }

        /// @copydoc IConnection::execute
        void execute(const std::string& query)
        {
            if (!m_db)
            {
                throw sqlite_error {SQLITE_CONNECTION_ERROR};
            }

            const auto result {sqlite3_exec(m_db.get(), query.c_str(), 0, 0, nullptr)};

            checkSqliteResult(result, query + ". " + sqlite3_errmsg(m_db.get()));
        }

        /// @copydoc IConnection::changes
        int64_t changes() const
        {
            return sqlite3_changes(m_db.get());
        }

    private:
        std::shared_ptr<sqlite3> m_db;
    };

    /// @brief SQLite transaction
    class Transaction : public ITransaction
    {
    public:
        /// @brief Virtual destructor
        ~Transaction()
        {
            try
            {
                if (!m_rolledBack && !m_commited)
                {
                    m_connection->execute("ROLLBACK TRANSACTION");
                }
            }
            // dtor should never throw
            catch (...)
            {
            }
        }

        /// @brief Constructor
        /// @param connection connection pointer
        explicit Transaction(std::shared_ptr<IConnection>& connection)
            : m_connection {connection}
            , m_rolledBack {false}
            , m_commited {false}
        {
            m_connection->execute("BEGIN TRANSACTION");
        }

        /// @copydoc ITransaction::commit
        void commit()
        {
            if (!m_rolledBack && !m_commited)
            {
                m_connection->execute("COMMIT TRANSACTION");
                m_commited = true;
            }
        }

        /// @copydoc ITransaction::rollback
        void rollback()
        {
            try
            {
                if (!m_rolledBack && !m_commited)
                {
                    m_rolledBack = true;
                    m_connection->execute("ROLLBACK TRANSACTION");
                }
            }
            // rollback can be called in a catch statement to unwind things so it shouldn't throw
            catch (...)
            {
            }
        }

        /// @brief Checks if the transaction is commited
        /// @return Value of m_commited
        bool isCommited() const
        {
            return m_commited;
        }

        /// @brief Checks if the transaction is rolled back
        /// @return Value of m_rolledBack
        bool isRolledBack() const
        {
            return m_rolledBack;
        }

    private:
        std::shared_ptr<IConnection> m_connection;
        bool m_rolledBack;
        bool m_commited;
    };

    /// @brief SQLite column
    class Column : public IColumn
    {
    public:
        /// @brief Virtual destructor
        ~Column() = default;

        /// @brief Constructor
        /// @param stmt statement
        /// @param index column index
        Column(std::shared_ptr<sqlite3_stmt>& stmt, const int32_t index)
            : m_stmt {stmt}
            , m_index {index}
        {
        }

        /// @copydoc IColumn::hasValue
        bool hasValue() const
        {
            return SQLITE_NULL != sqlite3_column_type(m_stmt.get(), m_index);
        }

        /// @copydoc IColumn::type
        int32_t type() const
        {
            return sqlite3_column_type(m_stmt.get(), m_index);
        }

        /// @copydoc IColumn::name
        std::string name() const
        {
            return sqlite3_column_name(m_stmt.get(), m_index);
        }

        /// @copydoc IColumn::value
        int32_t value(const int32_t&) const
        {
            return sqlite3_column_int(m_stmt.get(), m_index);
        }

        /// @copydoc IColumn::value
        uint64_t value(const uint64_t&) const
        {
            return sqlite3_column_int64(m_stmt.get(), m_index);
        }

        /// @copydoc IColumn::value
        int64_t value(const int64_t&) const
        {
            return sqlite3_column_int64(m_stmt.get(), m_index);
        }

        /// @copydoc IColumn::value
        double_t value(const double_t&) const
        {
            return sqlite3_column_double(m_stmt.get(), m_index);
        }

        /// @copydoc IColumn::value
        std::string value(const std::string&) const
        {
            const auto str {reinterpret_cast<const char*>(sqlite3_column_text(m_stmt.get(), m_index))};
            return nullptr != str ? str : "";
        }

    private:
        std::shared_ptr<sqlite3_stmt> m_stmt;
        const int32_t m_index;
    };

    /// @brief SQLite statement
    class Statement : public IStatement
    {
    public:
        /// @brief Virtual destructor
        ~Statement() = default;

        /// @brief Constructor
        /// @param connection connection pointer
        /// @param query query string
        Statement(std::shared_ptr<IConnection>& connection, const std::string& query)
            : m_connection {connection}
            , m_stmt {prepareSQLiteStatement(m_connection, query),
                      [](sqlite3_stmt* p)
                      {
                          sqlite3_finalize(p);
                      }}
            , m_bindParametersCount {sqlite3_bind_parameter_count(m_stmt.get())}
            , m_bindParametersIndex {0}
        {
        }

        /// @copydoc IStatement::step
        int32_t step()
        {
            auto ret {SQLITE_ERROR};

            if (m_bindParametersIndex == m_bindParametersCount)
            {
                ret = sqlite3_step(m_stmt.get());

                if (SQLITE_ROW != ret && SQLITE_DONE != ret)
                {
                    checkSqliteResult(ret, sqlite3_errmsg(m_connection->db().get()));
                }
            }

            return ret;
        }

        /// @copydoc IStatement::reset
        void reset()
        {
            sqlite3_reset(m_stmt.get());
            m_bindParametersIndex = 0;
        }

        /// @copydoc IStatement::bind
        void bind(const int32_t index, const int32_t value)
        {
            const auto result {sqlite3_bind_int(m_stmt.get(), index, value)};
            checkSqliteResult(result, sqlite3_errmsg(m_connection->db().get()));
            ++m_bindParametersIndex;
        }

        /// @copydoc IStatement::bind
        void bind(const int32_t index, const uint64_t value)
        {
            const auto result {sqlite3_bind_int64(m_stmt.get(), index, value)};
            checkSqliteResult(result, sqlite3_errmsg(m_connection->db().get()));
            ++m_bindParametersIndex;
        }

        /// @copydoc IStatement::bind
        void bind(const int32_t index, const int64_t value)
        {
            const auto result {sqlite3_bind_int64(m_stmt.get(), index, value)};
            checkSqliteResult(result, sqlite3_errmsg(m_connection->db().get()));
            ++m_bindParametersIndex;
        }

        /// @copydoc IStatement::bind
        void bind(const int32_t index, const std::string& value)
        {
            const auto result {sqlite3_bind_text(m_stmt.get(), index, value.c_str(), value.length(), SQLITE_TRANSIENT)};
            checkSqliteResult(result, sqlite3_errmsg(m_connection->db().get()));
            ++m_bindParametersIndex;
        }

        /// @copydoc IStatement::bind
        void bind(const int32_t index, const double_t value)
        {
            const auto result {sqlite3_bind_double(m_stmt.get(), index, value)};
            checkSqliteResult(result, sqlite3_errmsg(m_connection->db().get()));
            ++m_bindParametersIndex;
        }

        /// @copydoc IStatement::expand
        std::string expand()
        {
            return ExpandedSQLPtr(sqlite3_expanded_sql(m_stmt.get())).get();
        }

        /// @copydoc IStatement::column
        std::unique_ptr<IColumn> column(const int32_t index)
        {
            return std::make_unique<SQLiteLegacy::Column>(m_stmt, index);
        }

        /// @copydoc IStatement::columnsCount
        int columnsCount() const
        {
            return sqlite3_column_count(m_stmt.get());
        }

    private:
        std::shared_ptr<IConnection> m_connection;
        std::shared_ptr<sqlite3_stmt> m_stmt;
        const int m_bindParametersCount;
        int m_bindParametersIndex;
    };
} // namespace SQLiteLegacy
