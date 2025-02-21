#include <process_options.hpp>

#include <fmt/format.h>
#include <instance_handler.hpp>

#include <iostream>
#include <string>

void StatusAgent(const std::string& configFilePath)
{
    std::cout << fmt::format("wazuh-agent status: {}\n", instance_handler::GetAgentStatus(configFilePath));
}
