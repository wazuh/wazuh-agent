#pragma once

#include <agent_info.hpp>
#include <communicator.hpp>
#include <configuration_parser.hpp>
#include <multitype_queue.hpp>
#include <signal_handler.hpp>
#include <task_manager.hpp>

#include <string>

class Agent
{
public:
    Agent();
    ~Agent();

    void Run();

private:
    MultiTypeQueue m_messageQueue;

    SignalHandler m_signalHandler;
    TaskManager m_taskManager;
    AgentInfo m_agentInfo;
    configuration::ConfigurationParser m_configurationParser;
    communicator::Communicator m_communicator;
};
