#include <agent.hpp>
#include <message_task.hpp>

#include <chrono>
#include <thread>

Agent::Agent()
    : m_configurationParser(std::make_unique<configuration::ConfigurationParser>())
    , m_communicator(m_agentInfo.GetUUID(),
                     [this](std::string table, std::string key) -> std::string
                     { return m_configurationParser->GetConfig<std::string>(table, key); })
{
    m_taskManager.start(std::thread::hardware_concurrency());

    m_taskManager.enqueueTask(m_communicator.WaitForTokenExpirationAndAuthenticate());
    m_taskManager.enqueueTask(m_communicator.GetCommandsFromManager());

    std::string managerIp = m_configurationParser->GetConfig<std::string>("agent", "manager_ip");
    std::string port = m_configurationParser->GetConfig<std::string>("agent", "port");
    m_taskManager.enqueueTask(
        StatefulMessageProcessingTask(managerIp, port, m_communicator.GetToken(), m_messageQueue));
    m_taskManager.enqueueTask(
        StatelessMessageProcessingTask(managerIp, port, m_communicator.GetToken(), m_messageQueue));
}

Agent::~Agent()
{
    m_taskManager.stop();
}
