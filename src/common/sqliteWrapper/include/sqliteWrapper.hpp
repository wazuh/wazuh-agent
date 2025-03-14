#pragma once

#include "isqliteWrapper.hpp"
#include <memory>
#include <string>

/// @brief Forward declaration
struct sqlite3;
struct sqlite3_stmt;

namespace SQLiteLegacy
{
    /// @brief SQLite connection
    class Connection : public IConnection
    {
    public:
        /// @brief Default constructor
        Connection();

        /// @brief Virtual destructor
        ~Connection() = default;

        /// @brief Constructor
        /// @param path database path
        explicit Connection(const std::string& path);

        /// @copydoc IConnection::execute
        void execute(const std::string& query) override;

        /// @copydoc IConnection::close
        void close() override;

        /// @copydoc IConnection::db
        const std::shared_ptr<sqlite3>& db() const override;

        /// @copydoc IConnection::changes
        int64_t changes() const override;

    private:
        std::shared_ptr<sqlite3> m_db;
    };

    /// @brief SQLite transaction
    class Transaction : public ITransaction
    {
    public:
        /// @brief Virtual destructor
        ~Transaction();

        /// @brief Constructor
        /// @param connection connection pointer
        explicit Transaction(std::shared_ptr<IConnection>& connection);

        /// @copydoc ITransaction::commit
        void commit() override;

        /// @copydoc ITransaction::rollback
        void rollback() override;

        /// @brief Checks if the transaction is rolled back
        bool isRolledBack() const;

        /// @brief Checks if the transaction is commited
        bool isCommited() const;

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
        Column(std::shared_ptr<sqlite3_stmt>& stmt, const int32_t index);

        /// @copydoc IColumn::hasValue
        bool hasValue() const override;

        /// @copydoc IColumn::type
        int32_t type() const override;

        /// @copydoc IColumn::name
        std::string name() const override;

        /// @brief Returns the column value as int32_t
        /// @return Column value
        int32_t value(const int32_t&) const override;

        /// @brief Returns the column value as uint64_t
        /// @return Column value
        uint64_t value(const uint64_t&) const override;

        /// @brief Returns the column value as int64_t
        /// @return Column value
        int64_t value(const int64_t&) const override;

        /// @brief Returns the column value as string
        /// @return Column value
        std::string value(const std::string&) const override;

        /// @brief Returns the column value as double_t
        /// @return Column value
        double_t value(const double_t&) const override;

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
        Statement(std::shared_ptr<IConnection>& connection, const std::string& query);

        /// @copydoc IStatement::step
        int32_t step() override;

        /// @copydoc IStatement::reset
        void reset() override;

        /// @brief Binds a value
        /// @param index index of the column
        /// @param value value as int32_t
        void bind(const int32_t index, const int32_t value) override;

        /// @brief Binds a value
        /// @param index index of the column
        /// @param value value as uint64_t
        void bind(const int32_t index, const uint64_t value) override;

        /// @brief Binds a value
        /// @param index index of the column
        /// @param value value as int64_t
        void bind(const int32_t index, const int64_t value) override;

        /// @brief Binds a value
        /// @param index index of the column
        /// @param value value as string
        void bind(const int32_t index, const std::string& value) override;

        /// @brief Binds a value
        /// @param index index of the column
        /// @param value value as double_t
        void bind(const int32_t index, const double_t value) override;

        /// @brief Binds a value
        /// @param index index of the column
        void bind(const int32_t index) override;

        /// @copydoc IStatement::columnsCount
        int columnsCount() const override;

        /// @copydoc IStatement::expand
        std::string expand() override;

        /// @copydoc IStatement::column
        std::unique_ptr<IColumn> column(const int32_t index) override;

    private:
        std::shared_ptr<IConnection> m_connection;
        std::shared_ptr<sqlite3_stmt> m_stmt;
        const int m_bindParametersCount;
        int m_bindParametersIndex;
    };
} // namespace SQLiteLegacy
