#include <process_options.hpp>

/****************************************************
 *  Placeholder file. To be replaced on branch merge
 ****************************************************/

#include <agent.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <logger.hpp>

#include <vector>

void RestartAgent() {}

void StartAgent()
{
    Agent agent;
    agent.Run();
}

void StatusAgent() {}

void StopAgent()
{
    LogInfo("Stopping Wazuh Agent.");
}

void InstallService([[maybe_unused]] const std::string& exePath) {}

void RemoveService() {}

void SetDispatcherThread() {}
