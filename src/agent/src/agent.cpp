#include <agent.hpp>

#include <thread>

Agent::Agent()
    : m_communicator(m_agentInfo.GetUUID(),
                     [this](std::string table, std::string key) -> std::string
                     { return m_configurationParser.GetConfig<std::string>(table, key); })
{
    m_taskManager.Start(std::thread::hardware_concurrency());

    m_taskManager.EnqueueTask(m_communicator.WaitForTokenExpirationAndAuthenticate());
    m_taskManager.EnqueueTask(m_communicator.GetCommandsFromManager());
    m_taskManager.EnqueueTask(m_communicator.StatefulMessageProcessingTask(m_messageQueue));
    m_taskManager.EnqueueTask(m_communicator.StatelessMessageProcessingTask(m_messageQueue));
}

Agent::~Agent()
{
    m_communicator.Stop();
    m_taskManager.Stop();
}
