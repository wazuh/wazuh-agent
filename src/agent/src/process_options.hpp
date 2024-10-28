#pragma once

#include <string>

void RegisterAgent(const std::string& user,
                   const std::string& password,
                   const std::string& key,
                   const std::string& name,
                   const std::string& configFile);
void RestartAgent([[maybe_unused]] const std::string& configFile);
void StartAgent([[maybe_unused]] const std::string& configFile);
void StartAgentDaemon([[maybe_unused]] const std::string& configFile);
void StatusAgent();
void StopAgent();
void PrintHelp();
bool InstallService();
bool RemoveService();
void SetDispatcherThread();
