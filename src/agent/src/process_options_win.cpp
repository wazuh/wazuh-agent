#include <process_options.hpp>

#include <agent.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <logger.hpp>
#include <windows_service.hpp>

#include <vector>

#include <iostream> // remove

// void StartAgent(const std::string& configFilePath)
// {
//     std::cout << fmt::format("StartAgent: configFilePath = {}", configFilePath.c_str()).c_str() << std::endl;
//     WindowsService::ServiceStart(configFilePath);
// }

void StartAgent(const std::string& configFilePath)
{
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

void StartAgentService(const std::string& configFilePath)
{
    WindowsService::ServiceStart(configFilePath);
}

void StatusAgent([[maybe_unused]] const std::string& configFilePath)
{
    WindowsService::ServiceStatus();
}

bool InstallService()
{
    windows_api_facade::WindowsApiFacade windowsApiFacade;
    return WindowsService::InstallService(windowsApiFacade);
}

bool RemoveService()
{
    windows_api_facade::WindowsApiFacade windowsApiFacade;
    return WindowsService::RemoveService(windowsApiFacade);
}

void SetDispatcherThread()
{
    WindowsService::SetDispatcherThread();
}
