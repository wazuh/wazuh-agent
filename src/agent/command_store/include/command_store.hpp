#pragma once

#include <module_command/command_entry.hpp>

#include <memory>
#include <optional>
#include <string>

class Persistence;

namespace command_store
{
    /// @brief CommandStore class
    ///
    /// This class provides methods for storing, retrieving, and deleting commands
    /// in the command store database. It uses a database to store the
    /// commands.
    class CommandStore
    {
    private:
        /// @brief Unique pointer to the persistence instance.
        std::unique_ptr<Persistence> m_dataBase;

        /// @brief Gets the current timestamp in seconds
        /// @return The current timestamp in seconds
        double GetCurrentTimestampAsReal();

        /// @brief Converts an integer to a module_command::Status value
        /// @param i The integer to convert
        /// @return The module_command::Status value corresponding
        module_command::Status StatusFromInt(const int i);

        /// @brief Converts an integer to a module_command::CommandExecutionMode value
        /// @param i The integer to convert
        /// @return The module_command::CommandExecutionMode value corresponding
        module_command::CommandExecutionMode ExecutionModeFromInt(const int i);

    public:
        /// @brief CommandStore constructor
        /// @param dbFolderPath The path to the database folder
        CommandStore(const std::string& dbFolderPath);

        /// @brief CommandStore destructor
        ~CommandStore();

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

        /// @brief Retrieves a vector of commands from the database by status
        /// @param status The status of the commands to retrieve
        /// @return An optional containing the commands if found, or nullopt if not
        std::optional<std::vector<module_command::CommandEntry>>
        GetCommandByStatus(const module_command::Status& status);

        /// @brief Updates an existing command in the database
        /// @param cmd The updated command data
        /// @return True if successful, false otherwise
        bool UpdateCommand(const module_command::CommandEntry& cmd);
    };
} // namespace command_store
