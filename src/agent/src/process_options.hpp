#pragma once

#include <string>
#include <vector>

/// @brief Registers the agent with the given parameters.
/// @param url The Server Management API URL.
/// @param user The user to use for authentication with Server Management API.
/// @param password The password to use for authentication with Server Management API.
/// @param key The key to use for registration.
/// @param name The name to use for the agent.
/// @param configFile The path to the configuration file.
void RegisterAgent(const std::string& url,
                   const std::string& user,
                   const std::string& password,
                   const std::string& key,
                   const std::string& name,
                   const std::string& configFile);

/// @brief Starts the agent using the specified configuration file.
/// @param configFile The file path to the configuration file used for starting the agent.
void StartAgent(const std::string& configFile);

/// @brief Displays the current status of the agent.
/// @param configFile The file path to the configuration file used to get the status of the agent.
void StatusAgent(const std::string& configFile);

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
