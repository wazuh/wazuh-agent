#pragma once

#include <limits>
#include <string>

namespace command_store
{
    enum class Status
    {
        SUCCESS,
        FAILURE,
        IN_PROGRESS,
        TIMEOUT,
        UNKNOWN
    };

    class Command
    {
    public:
        Command()
            : m_status(Status::UNKNOWN)
            , m_time(0.0)
        {
        }

        Command(const std::string& id,
                const std::string& module,
                const std::string& command,
                const std::string& parameters,
                const std::string& result,
                Status status)
            : m_id(id)
            , m_module(module)
            , m_command(command)
            , m_parameters(parameters)
            , m_result(result)
            , m_status(status)
        {
        }

        std::string m_id;
        std::string m_module;
        std::string m_command;
        std::string m_parameters;
        std::string m_result;
        Status m_status;
        double m_time;
    };
} // namespace command_store
