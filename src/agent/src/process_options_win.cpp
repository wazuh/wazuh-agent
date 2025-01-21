#include <process_options.hpp>

#include <agent.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <logger.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <windows_service.hpp>

#include <memory>
#include <vector>

void StartAgentService(const std::string& configFilePath)
{
    WindowsService::ServiceStart(configFilePath);
}

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

        Agent agent(configFilePath);
        agent.Run();
    }
    catch (const std::exception& e)
    {
        LogError("Exception thrown in wazuh-agent: {}", e.what());
    }
}

void StatusAgent([[maybe_unused]] const std::string& configFilePath)
{
    // TODO: implement our own status function using lock files
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
