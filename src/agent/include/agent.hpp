#pragma once

#include <agent_info.hpp>
#include <centralized_configuration.hpp>
#include <command_handler.hpp>
#include <communicator.hpp>
#include <configuration_parser.hpp>
#include <isignal_handler.hpp>
#include <moduleManager.hpp>
#include <multitype_queue.hpp>
#include <signal_handler.hpp>
#include <task_manager.hpp>

#include <sysInfo.hpp>

#include <memory>
#include <string>

/// @brief Agent class
///
/// This class handles the configuration, communication with the manager,
/// command handling, task management, and module management.
class Agent
{
public:
    /// @brief Constructor
    /// @param configFile Path to the configuration file
    /// @param signalHandler Pointer to a custom ISignalHandler implementation
    Agent(const std::string& configFile,
          std::unique_ptr<ISignalHandler> signalHandler = std::make_unique<SignalHandler>());

    /// @brief Destructor
    ~Agent();

    /// @brief Runs the agent
    ///
    /// This method sets up the agent and starts the task manager.
    void Run();

private:
    /// @brief Task manager
    TaskManager m_taskManager;

    /// @brief System info
    SysInfo m_sysInfo;

    /// @brief Configuration parser
    configuration::ConfigurationParser m_configurationParser;

    std::string m_dataPath;

    /// @brief Queue for storing messages
    std::shared_ptr<MultiTypeQueue> m_messageQueue;

    /// @brief Pointer to a custom ISignalHandler implementation
    std::unique_ptr<ISignalHandler> m_signalHandler;

    /// @brief Agent info
    AgentInfo m_agentInfo;

    /// @brief Communicator
    communicator::Communicator m_communicator;

    /// @brief Module manager
    ModuleManager m_moduleManager;

    /// @brief Command handler
    command_handler::CommandHandler m_commandHandler;

    /// @brief Centralized configuration
    centralized_configuration::CentralizedConfiguration m_centralizedConfiguration;
};
