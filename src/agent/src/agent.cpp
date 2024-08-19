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
        [this]() { return getMessagesFromQueue(m_messageQueue, STATEFUL); },
        [this]([[maybe_unused]] const std::string& response) { popMessagesFromQueue(m_messageQueue, STATEFUL); }));

    m_taskManager.EnqueueTask(m_communicator.StatelessMessageProcessingTask(
        [this]() { return getMessagesFromQueue(m_messageQueue, STATELESS); },
        [this]([[maybe_unused]] const std::string& response) { popMessagesFromQueue(m_messageQueue, STATELESS); }));

    m_taskManager.EnqueueTask(PushTestMessages());

    m_taskManager.EnqueueTask(m_commandManager.ProcessCommandsFromQueue<Message>(
        [this]() -> std::optional<Message>
        {
            while (m_agentQueue.isEmpty(MessageType::COMMAND))
            {
                return std::nullopt;
            }

            Message m = m_agentQueue.getNext(MessageType::COMMAND);

            std::cout << "--------- COMMAND retrieved from Queue:" << std::endl;
            std::cout << m.data.dump() << std::endl;

            // pop message from queue
            m_agentQueue.pop(MessageType::COMMAND);

            // change status and push to CommandStore
            Message newMessage(MessageType::COMMAND, m.data, "CommandManager");
            setJsonValue(newMessage.data, "status", "InProgress");
            std::cout << newMessage.data.dump() << std::endl;
            // m_agentQueue.push(newMessage);

            return m;
        },
        [this](Message& cmd) -> int
        {
            std::cout << "Dispatching command" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return 1;
        }));

    m_signalHandler.WaitForSignal();
}
