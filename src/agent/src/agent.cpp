#include <agent.hpp>

#include <shared.hpp>

#include <string>
#include <thread>

namespace
{
    template<MessageType messageType>
    auto getMessagesFromQueue(MultiTypeQueue& multiTypeQueue)
    {
        return [&multiTypeQueue]() -> std::string
        {
            const auto message = multiTypeQueue.getLastMessage(messageType);
            return message.data.dump();
        };
    }
} // namespace

Agent::Agent()
    : m_communicator(m_agentInfo.GetUUID(),
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
    m_taskManager.EnqueueTask(m_communicator.GetCommandsFromManager(getMessagesFromQueue<COMMAND>(m_messageQueue)));
    m_taskManager.EnqueueTask(
        m_communicator.StatefulMessageProcessingTask(getMessagesFromQueue<STATEFUL>(m_messageQueue)));
    m_taskManager.EnqueueTask(
        m_communicator.StatelessMessageProcessingTask(getMessagesFromQueue<STATELESS>(m_messageQueue)));

    m_signalHandler.WaitForSignal();
}
