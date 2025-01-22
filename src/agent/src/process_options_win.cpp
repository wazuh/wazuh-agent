#include <process_options.hpp>

#include <agent.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <instance_handler.hpp>
#include <logger.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <windows_service.hpp>

#include <iostream>
#include <memory>
#include <vector>

void StartAgent(const std::string& configFilePath)
{
    try
    {
        std::shared_ptr<spdlog::logger> logger = spdlog::get("wazuh-agent");
        if (logger != nullptr)
        {
            auto stdOutSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
            logger->sinks().clear();
            logger->sinks().push_back(stdOutSink);
        }

        instance_handler::InstanceHandler instanceHandler = instance_handler::GetInstanceHandler(configFilePath);

        if (!instanceHandler.isLockAcquired())
        {
            std::cout << "wazuh-agent already running\n";
            return;
        }

        LogInfo("Starting wazuh-agent");
        Agent agent(configFilePath);
        agent.Run();
    }
    catch (const std::exception& e)
    {
        LogError("Exception thrown in wazuh-agent: {}", e.what());
    }
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
