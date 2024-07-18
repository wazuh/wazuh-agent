#pragma once

#include <communicator.hpp>
#include <configuration_parser.hpp>
#include <task_manager.hpp>

#include <queue>
#include <string>

class Agent
{
public:
    Agent();
    ~Agent();

    const communicator::Communicator& GetCommunicator()
    {
        return m_communicator;
    }

private:
    std::queue<std::string> m_messageQueue;

    TaskManager m_taskManager;
    configuration::ConfigurationParser m_configurationParser;
    communicator::Communicator m_communicator;
};
