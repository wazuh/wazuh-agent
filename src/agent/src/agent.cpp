#include <agent.hpp>

#include <thread>

Agent::Agent()
    : m_communicator(m_agentInfo.GetUUID(),
                     m_agentInfo.GetKey(),
                     [this](std::string table, std::string key) -> std::string
                     { return m_configurationParser.GetConfig<std::string>(table, key); })
{
    m_taskManager.Start(std::thread::hardware_concurrency());
}

Agent::~Agent()
{
    m_taskManager.Stop();
}

void Agent::Run()
{
    m_taskManager.EnqueueTask(m_communicator.WaitForTokenExpirationAndAuthenticate());
    m_taskManager.EnqueueTask(m_communicator.GetCommandsFromManager(m_messageQueue));
    m_taskManager.EnqueueTask(m_communicator.StatefulMessageProcessingTask(m_messageQueue));
    m_taskManager.EnqueueTask(m_communicator.StatelessMessageProcessingTask(m_messageQueue));

    m_signalHandler.WaitForSignal();
}
