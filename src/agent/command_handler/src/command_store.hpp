#pragma once

#include <command_entry.hpp>
#include <icommand_store.hpp>
#include <persistence.hpp>

#include <memory>
#include <optional>
#include <string>

namespace command_store
{
    /// @brief CommandStore class
    ///
    /// This class provides methods for storing, retrieving, and deleting commands
    /// in the command store database. It uses a database to store the
    /// commands.
    class CommandStore : public ICommandStore
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
        /// @param persistence Optional pointer to an existing persistence object.
        CommandStore(const std::string& dbFolderPath, std::unique_ptr<Persistence> persistence = nullptr);

        /// @brief CommandStore destructor
        ~CommandStore();

        /// @copydoc ICommandStore::Clear
        bool Clear() override;

        /// @copydoc ICommandStore::GetCount
        int GetCount() override;

        /// @copydoc ICommandStore::StoreCommand
        bool StoreCommand(const module_command::CommandEntry& cmd) override;

        /// @copydoc ICommandStore::DeleteCommand
        bool DeleteCommand(const std::string& id) override;

        /// @copydoc ICommandStore::GetCommand
        std::optional<module_command::CommandEntry> GetCommand(const std::string& id) override;

        /// @copydoc ICommandStore::GetCommandByStatus
        std::optional<std::vector<module_command::CommandEntry>>
        GetCommandByStatus(const module_command::Status& status) override;

        /// @copydoc ICommandStore::UpdateCommand
        bool UpdateCommand(const module_command::CommandEntry& cmd) override;
    };
} // namespace command_store
