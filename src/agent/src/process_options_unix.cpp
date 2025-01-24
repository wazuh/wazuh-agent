#include <process_options.hpp>

#include <agent.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <instance_handler.hpp>
#include <logger.hpp>

#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

void StartAgent(const std::string& configFilePath)
{
    instance_handler::InstanceHandler instanceHandler = instance_handler::GetInstanceHandler(configFilePath);

    if (!instanceHandler.isLockAcquired())
    {
        std::cout << "wazuh-agent already running\n";
        return;
    }

    LogInfo("Starting wazuh-agent");

    try
    {
        Agent agent(configFilePath);
        agent.Run();
    }
    catch (const std::exception& e)
    {
        LogError("Exception thrown in wazuh-agent: {}", e.what());
    }
}
