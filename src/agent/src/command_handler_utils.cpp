#include <command_handler_utils.hpp>

#include <iostream>

std::tuple<command_store::Status, std::string> DispatchCommand(const command_store::Command& cmd)
{
    // TO_DO: Implement real dispatch function.
    std::cout << "Dispatching command " << cmd.m_module << "\n";
    return {command_store::Status::SUCCESS, "Successfully executed."};
}
