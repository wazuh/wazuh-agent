#include <agent.hpp>
#include <message_task.hpp>

#include <chrono>
#include <thread>

Agent::Agent()
: m_configurationParser(std::make_shared<configuration::ConfigurationParser>())
, m_communicator([this](std::string table, std::string key)->std::string { return m_configurationParser->GetConfig<std::string>(table, key);})
{
    m_taskManager.start(std::thread::hardware_concurrency());

    m_taskManager.enqueueTask(m_communicator.WaitForTokenExpirationAndAuthenticate());

    m_taskManager.enqueueTask(StatefulMessageProcessingTask(m_communicator.GetToken(), m_messageQueue));
    m_taskManager.enqueueTask(StatelessMessageProcessingTask(m_communicator.GetToken(), m_messageQueue));
}

Agent::~Agent()
{
    m_communicator.stop();
    m_taskManager.stop();
}
