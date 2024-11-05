#pragma once

#include <string>

/// Command-line options
static const auto OPT_RUN {"--run"};
static const auto OPT_START {"--start"};
static const auto OPT_STATUS {"--status"};
static const auto OPT_STOP {"--stop"};
static const auto OPT_RESTART {"--restart"};
static const auto OPT_CONFIG_FILE {"--config-file"};
static const auto OPT_REGISTER_AGENT {"--register-agent"};
static const auto OPT_INSTALL_SERVICE {"--install-service"};
static const auto OPT_REMOVE_SERVICE {"--remove-service"};
static const auto OPT_RUN_SERVICE {"--run-service"};
static const auto OPT_USER {"--user"};
static const auto OPT_PASSWORD {"--password"};
static const auto OPT_KEY {"--key"};
static const auto OPT_NAME {"--name"};
static const auto OPT_HELP {"--help"};

/// @brief Registers the agent with the given parameters.
/// @param user The user to use for authentication with Server Management API.
/// @param password The password to use for authentication with Server Management API.
/// @param key The key to use for registration.
/// @param name The name to use for the agent.
/// @param configFile The file path to the configuration file.
void RegisterAgent(const std::string& user,
                   const std::string& password,
                   const std::string& key,
                   const std::string& name,
                   const std::string& configFile);

/// @brief Restarts the agent using the specified configuration file.
/// @param configFile The file path to the configuration file to use for restarting the agent.
void RestartAgent(const std::string& configFile);

/// @brief Starts the agent using the specified configuration file.
/// @param configFile The file path to the configuration file to use for starting the agent.
void StartAgent(const std::string& configFile);

/// @brief Displays the current status of the agent.
void StatusAgent();

/// @brief Stops the agent.
void StopAgent();

/// @brief Prints the help message with usage instructions.
void PrintHelp();

/// @brief Installs the agent as a service.
/// @return True if the installation is successful, false otherwise.
bool InstallService();

/// @brief Removes the agent service.
/// @return True if the removal is successful, false otherwise.
bool RemoveService();

/// @brief Sets up the dispatcher thread for the agent.
void SetDispatcherThread();
