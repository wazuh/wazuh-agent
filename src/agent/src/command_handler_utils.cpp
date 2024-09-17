#include <command_handler_utils.hpp>
#include <logger.hpp>

std::tuple<command_store::Status, std::string> DispatchCommand(const command_store::Command& cmd)
{
    // TO_DO: Implement real dispatch function.
    LogInfo("Dispatching command {}({})", cmd.m_command, cmd.m_module);
    return {command_store::Status::SUCCESS, "Successfully executed."};
}
