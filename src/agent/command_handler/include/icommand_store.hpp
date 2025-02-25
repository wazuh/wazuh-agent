#pragma once

#include <command_entry.hpp>

#include <optional>
#include <string>
#include <vector>

namespace command_store
{
    /// @brief Interface for CommandStore
    class ICommandStore
    {
    public:
        /// @brief Virtual destructor
        virtual ~ICommandStore() = default;

        /// @brief Clears all commands from the database
        /// @return True if successful, false otherwise
        virtual bool Clear() = 0;

        /// @brief Gets the count of commands in the database
        /// @return The number of commands in the database
        virtual int GetCount() = 0;

        /// @brief Stores a command in the database
        /// @param cmd The command to store
        /// @return True if successful, false otherwise
        virtual bool StoreCommand(const module_command::CommandEntry& cmd) = 0;

        /// @brief Deletes a command from the database by ID
        /// @param id The ID of the command to delete
        /// @return True if successful, false otherwise
        virtual bool DeleteCommand(const std::string& id) = 0;

        /// @brief Retrieves a command from the database by ID
        /// @param id The ID of the command to retrieve
        /// @return An optional containing the command if found, or nullopt if not
        virtual std::optional<module_command::CommandEntry> GetCommand(const std::string& id) = 0;

        /// @brief Retrieves a vector of commands from the database by status
        /// @param status The status of the commands to retrieve
        /// @return An optional containing the commands if found, or nullopt if not
        virtual std::optional<std::vector<module_command::CommandEntry>>
        GetCommandByStatus(const module_command::Status& status) = 0;

        /// @brief Updates an existing command in the database
        /// @param cmd The updated command data
        /// @return True if successful, false otherwise
        virtual bool UpdateCommand(const module_command::CommandEntry& cmd) = 0;
    };
} // namespace command_store
