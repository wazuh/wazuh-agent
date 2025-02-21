#pragma once

#include <string>
#include <vector>

/// @brief Starts the agent using the specified configuration file.
/// @param configFilePath The file path to the configuration file used for starting the agent.
void StartAgent(const std::string& configFilePath);

/// @brief Displays the current status of the agent.
/// @param configFilePath The file path to the configuration file used to get the status of the agent.
void StatusAgent(const std::string& configFilePath);

#ifdef _WIN32
/// @brief Installs the agent as a service.
/// @return True if the installation is successful, false otherwise.
bool InstallService();

/// @brief Removes the agent service.
/// @return True if the removal is successful, false otherwise.
bool RemoveService();

/// @brief Sets up the dispatcher thread for the agent.
void SetDispatcherThread();
#endif
