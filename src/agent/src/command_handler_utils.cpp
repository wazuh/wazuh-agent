#include <command_handler_utils.hpp>
#include <logger.hpp>

std::tuple<command_store::Status, std::string> DispatchCommand(const command_store::CommandEntry& cmd)
{
    // TO_DO: Implement real dispatch function.
    LogInfo("Dispatching command {}({})", cmd.Command, cmd.Module);
    return {command_store::Status::SUCCESS, "Successfully executed"};
}
