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

    std::string managerIp = m_configurationParser->GetConfig<std::string>("agent", "manager_ip");
    std::string port = m_configurationParser->GetConfig<std::string>("agent", "port");
    m_taskManager.EnqueueTask(
        m_communicator.StatefulMessageProcessingTask(managerIp, port, m_communicator.GetToken(), m_messageQueue));
    m_taskManager.EnqueueTask(
        m_communicator.StatelessMessageProcessingTask(managerIp, port, m_communicator.GetToken(), m_messageQueue));
}

Agent::~Agent()
{
    m_taskManager.Stop();
}
