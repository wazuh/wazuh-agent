#pragma once

#include <agent_info.hpp>
#include <communicator.hpp>
#include <configuration_parser.hpp>
#include <isignal_handler.hpp>
#include <multitype_queue.hpp>
#include <signal_handler.hpp>
#include <task_manager.hpp>

#include <memory>

class Agent
{
public:
    Agent(std::unique_ptr<ISignalHandler> signalHandler = std::make_unique<SignalHandler>());
    ~Agent();

    void Run();

private:
    MultiTypeQueue m_messageQueue;

    std::unique_ptr<ISignalHandler> m_signalHandler;
    TaskManager m_taskManager;
    AgentInfo m_agentInfo;
    configuration::ConfigurationParser m_configurationParser;
    communicator::Communicator m_communicator;
};
