#pragma once

#include <string>

void RegisterAgent(const std::string& user,
                   const std::string& password,
                   const std::string& key,
                   const std::string& name);
void RestartAgent();
void StartAgent();
void StartAgentDaemon();
void StatusAgent();
void StopAgent();
void PrintHelp();
bool InstallService();
bool RemoveService();
void SetDispatcherThread();
