#pragma once

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

    class CommandEntry
    {
    public:
        CommandEntry()
            : CurrentStatus(Status::UNKNOWN)
            , Time(0.0)
        {
        }

        CommandEntry(const std::string& id,
                     const std::string& module,
                     const std::string& command,
                     const std::string& parameters,
                     const std::string& result,
                     Status status)
            : Id(id)
            , Module(module)
            , Command(command)
            , Parameters(parameters)
            , Result(result)
            , CurrentStatus(status)
        {
        }

        std::string Id;
        std::string Module;
        std::string Command;
        std::string Parameters;
        std::string Result;
        Status CurrentStatus;
        double Time;
    };
} // namespace module_command
