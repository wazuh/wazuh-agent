#pragma once

#include <string>

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
