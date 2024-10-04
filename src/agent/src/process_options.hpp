#pragma once

#include <string>

void RegisterAgent(const std::string& user,
                   const std::string& password,
                   const std::string& key,
                   const std::string& name);
void RestartAgent();
void StartAgent();
void StatusAgent();
void StopAgent();
void PrintHelp();
void InstallService([[maybe_unused]] const std::string& exePath);
void RemoveService();
void SetDispatcherThread();
