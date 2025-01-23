#pragma once

#include <nlohmann/json.hpp>

#include <limits>
#include <string>

namespace module_command
{
    /// @brief Commands
    const std::string SET_GROUP_COMMAND = "set-group";
    const std::string UPDATE_GROUP_COMMAND = "update-group";

    /// @brief Commands arguments
    const std::string GROUPS_ARG = "groups";

    /// @brief Modules
    const std::string CENTRALIZED_CONFIGURATION_MODULE = "CentralizedConfiguration";

    /// @enum Status of a command execution
    enum class Status
    {
        SUCCESS,
        FAILURE,
        IN_PROGRESS,
        TIMEOUT,
        UNKNOWN
    };

    /// @enum Execution mode of a command
    enum class CommandExecutionMode
    {
        SYNC,
        ASYNC
    };

    /// @struct Result of a command execution
    struct CommandExecutionResult
    {
        Status ErrorCode = Status::UNKNOWN;
        std::string Message;

        /// @brief Construct a CommandExecutionResult object
        /// @param code The status of the command execution
        /// @param message A message associated with the command execution result
        explicit CommandExecutionResult(Status code = Status::UNKNOWN, std::string message = "")
            : ErrorCode(code)
            , Message(std::move(message))
        {
        }
    };

    /// @brief Data structure to hold the command execution entry
    class CommandEntry
    {
    public:
        /// @brief Construct a CommandEntry object
        CommandEntry()
            : Time(0.0)
            , ExecutionResult(Status::UNKNOWN, "")
        {
        }

        /// @brief Construct a CommandEntry object
        /// @param id The identifier of the command entry
        /// @param module The module of the command
        /// @param command The command to be executed
        /// @param parameters The parameters for the command
        /// @param result The result of the command execution
        /// @param status The status of the command execution
        CommandEntry(std::string id,
                     std::string module,
                     std::string command,
                     nlohmann::json parameters,
                     CommandExecutionMode executionMode,
                     std::string result,
                     Status status)
            : Id(std::move(id))
            , Module(std::move(module))
            , Command(std::move(command))
            , Parameters(std::move(parameters))
            , Time(0.0)
            , ExecutionMode(executionMode)
            , ExecutionResult(status, std::move(result))
        {
        }

        /// @brief Unique identifier of the command entry
        std::string Id;

        /// @brief Module the command belongs to
        std::string Module;

        /// @brief Command to be executed
        std::string Command;

        /// @brief Parameters for the command
        nlohmann::json Parameters;

        /// @brief Time of the command execution
        double Time;

        /// @brief Execution mode (sync or async) of the command.
        CommandExecutionMode ExecutionMode;

        /// @brief Result of the command execution
        CommandExecutionResult ExecutionResult;
    };
} // namespace module_command
