#include <agent.hpp>

#include <message.hpp>
#include <message_queue_utils.hpp>

#include <string>
#include <thread>
#include <vector>

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

    m_taskManager.EnqueueTask(m_communicator.GetCommandsFromManager(
        [this](const std::string& response) { pushCommandsToQueue(m_messageQueue, response); }));

    m_taskManager.EnqueueTask(m_communicator.StatefulMessageProcessingTask(
        [this]() { return getMessagesFromQueue(m_messageQueue, MessageType::STATEFUL); },
        [this]([[maybe_unused]] const std::string& response)
        { popMessagesFromQueue(m_messageQueue, MessageType::STATEFUL); }));

    m_taskManager.EnqueueTask(m_communicator.StatelessMessageProcessingTask(
        [this]() { return getMessagesFromQueue(m_messageQueue, MessageType::STATELESS); },
        [this]([[maybe_unused]] const std::string& response)
        { popMessagesFromQueue(m_messageQueue, MessageType::STATELESS); }));

    m_signalHandler.WaitForSignal();
}
