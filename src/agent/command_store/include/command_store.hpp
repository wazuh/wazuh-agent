#pragma once

#include <module_command/command_entry.hpp>
#include <sqlite_manager.hpp>

#include <memory>
#include <optional>
#include <string>

namespace command_store
{
    const std::string COMMANDSTORE_DB_NAME = "command_store.db";
    const std::string COMMANDSTORE_TABLE_NAME = "COMMAND";

    /// @brief CommandStore class
    ///
    /// This class provides methods for storing, retrieving, and deleting commands
    /// in the command store database. It uses a SQLite database to store the
    /// commands.
    class CommandStore
    {
    private:
        /// @brief The SQLite database object
        std::unique_ptr<sqlite_manager::SQLiteManager> m_dataBase;

        /// @brief Gets the current timestamp in seconds
        /// @return The current timestamp in seconds
        double GetCurrentTimestampAsReal();

        /// @brief Converts an integer to a module_command::Status value
        /// @param i The integer to convert
        /// @return The module_command::Status value corresponding
        module_command::Status StatusFromInt(const int i);

    public:
        /// @brief CommandStore constructor
        /// @param dbFolderPath The path to the database folder
        CommandStore(const std::string& dbFolderPath);

        /// @brief Clears all commands from the database
        /// @return True if successful, false otherwise
        bool Clear();

        /// @brief Gets the count of commands in the database
        /// @return The number of commands in the database
        int GetCount();

        /// @brief Stores a command in the database
        /// @param cmd The command to store
        /// @return True if successful, false otherwise
        bool StoreCommand(const module_command::CommandEntry& cmd);

        /// @brief Deletes a command from the database by ID
        /// @param id The ID of the command to delete
        /// @return True if successful, false otherwise
        bool DeleteCommand(const std::string& id);

        /// @brief Retrieves a command from the database by ID
        /// @param id The ID of the command to retrieve
        /// @return An optional containing the command if found, or nullopt if not
        std::optional<module_command::CommandEntry> GetCommand(const std::string& id);

        /// @brief Updates an existing command in the database
        /// @param cmd The updated command data
        /// @return True if successful, false otherwise
        bool UpdateCommand(const module_command::CommandEntry& cmd);
    };
} // namespace command_store
