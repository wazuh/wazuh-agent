#pragma once

#include "sqliteWrapper.hpp"
#include <memory>
#include <string>

namespace SQLiteLegacy
{
    /// @brief SQLite factory interface
    class ISQLiteFactory
    {
    public:
        /// @brief Virtual destructor
        virtual ~ISQLiteFactory() = default;

        /// @brief Creates a connection
        /// @param path database path
        /// @return Connection pointer
        virtual std::shared_ptr<SQLiteLegacy::IConnection> createConnection(const std::string& path) = 0;

        /// @brief Creates a transaction
        /// @param connection connection pointer
        /// @return Transaction pointer
        virtual std::unique_ptr<SQLiteLegacy::ITransaction>
        createTransaction(std::shared_ptr<SQLiteLegacy::IConnection>& connection) = 0;

        /// @brief Creates a statement
        /// @param connection connection pointer
        /// @param query query string
        /// @return Statement pointer
        virtual std::unique_ptr<SQLiteLegacy::IStatement>
        createStatement(std::shared_ptr<SQLiteLegacy::IConnection>& connection, const std::string& query) = 0;
    };

    class SQLiteFactory : public ISQLiteFactory
    {
    public:
        /// @brief Default constructor
        SQLiteFactory() = default;

        /// @brief Virtual destructor
        ~SQLiteFactory() = default;

        /// @brief Delete copy constructor
        SQLiteFactory(const SQLiteFactory&) = delete;

        /// @brief Delete copy assignment
        SQLiteFactory& operator=(const SQLiteFactory&) = delete;

        /// @copydoc ISQLiteFactory::createConnection
        std::shared_ptr<SQLiteLegacy::IConnection> createConnection(const std::string& path) override
        {
            return std::make_shared<SQLiteLegacy::Connection>(path);
        }

        /// @copydoc ISQLiteFactory::createTransaction
        std::unique_ptr<SQLiteLegacy::ITransaction>
        createTransaction(std::shared_ptr<SQLiteLegacy::IConnection>& connection) override
        {
            return std::make_unique<SQLiteLegacy::Transaction>(connection);
        }

        /// @copydoc ISQLiteFactory::createStatement
        std::unique_ptr<SQLiteLegacy::IStatement>
        createStatement(std::shared_ptr<SQLiteLegacy::IConnection>& connection, const std::string& query) override
        {
            return std::make_unique<SQLiteLegacy::Statement>(connection, query);
        }
    };
} // namespace SQLiteLegacy
