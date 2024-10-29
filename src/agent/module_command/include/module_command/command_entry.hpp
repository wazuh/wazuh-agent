#pragma once

#include <nlohmann/json.hpp>

#include <limits>
#include <string>

namespace module_command
{
    enum class Status
    {
        SUCCESS,
        FAILURE,
        IN_PROGRESS,
        TIMEOUT,
        UNKNOWN
    };

    struct CommandExecutionResult
    {
        Status ErrorCode = Status::UNKNOWN;
        std::string Message;

        explicit CommandExecutionResult(Status code = Status::UNKNOWN, std::string message = "")
            : ErrorCode(code)
            , Message(std::move(message))
        {
        }
    };

    class CommandEntry
    {
    public:
        // Default constructor
        CommandEntry()
            : Time(0.0)
            , ExecutionResult(Status::UNKNOWN, "")
        {
        }

        CommandEntry(std::string id,
                     std::string module,
                     std::string command,
                     nlohmann::json parameters,
                     std::string result,
                     Status status)
            : Id(std::move(id))
            , Module(std::move(module))
            , Command(std::move(command))
            , Parameters(std::move(parameters))
            , Time(0.0)
            , ExecutionResult(status, std::move(result))
        {
        }

        std::string Id;
        std::string Module;
        std::string Command;
        nlohmann::json Parameters;
        double Time;
        CommandExecutionResult ExecutionResult;
    };
} // namespace module_command
