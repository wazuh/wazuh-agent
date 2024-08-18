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
    // add some test command to the queue
    // const nlohmann::json dataContent = {{"command", {{"name", "001"}, {"type", "stateless"}}},
    //                                    {"origin", {{"serverName", "node01"}, {"moduleName", "upgradeModule"}}},
    //                                    {"parameters", {{"error", 0}, {"data", "Command received"}}},
    //                                    {"status", "Pending"}};
    //

    m_taskManager.EnqueueTask(m_communicator.WaitForTokenExpirationAndAuthenticate());

    m_taskManager.EnqueueTask(m_communicator.GetCommandsFromManager(
        [this](const std::string& response) { pushCommandsToQueue(m_messageQueue, response); }));

    m_taskManager.EnqueueTask(m_communicator.StatefulMessageProcessingTask(
        [this]() { return getMessagesFromQueue(m_messageQueue, STATEFUL); },
        [this]([[maybe_unused]] const std::string& response) { popMessagesFromQueue(m_messageQueue, STATEFUL); }));

    m_taskManager.EnqueueTask(m_communicator.StatelessMessageProcessingTask(
        [this]() { return getMessagesFromQueue(m_messageQueue, STATELESS); },
        [this]([[maybe_unused]] const std::string& response) { popMessagesFromQueue(m_messageQueue, STATELESS); }));


    // Push message to Agent Queue every 100 seconds.
    m_taskManager.EnqueueTask([this]() -> boost::asio::awaitable<void> {

        while (true) {
            // Push the message
            const nlohmann::json dataContent = {{"command", "upgradeModule"}, {"status", "Pending"}};
            std::cout << "--------------------------------" << std::endl;
            std::cout << "Push command to the agent queue:" << std::endl;
            std::cout << dataContent << std::endl;
            std::cout << "--------------------------------" << std::endl;

            const Message messageToSend {MessageType::COMMAND, dataContent, "CommandManager"};
            m_agentQueue.push(messageToSend);
            std::this_thread::sleep_for(std::chrono::seconds(100));
        }
    });


    m_taskManager.EnqueueTask(m_commandManager.ProcessCommandsFromQueue<Message>(
        [this]() -> std::optional<Message>
        {
            if (m_agentQueue.isEmpty(MessageType::COMMAND))
            {
                return std::nullopt;
            }
            Message m = m_agentQueue.getNext(MessageType::COMMAND);

            std::cout << "COMMAND retrieved from Queue:" << std::endl;

            // pop message from queue
            m_agentQueue.pop(MessageType::COMMAND);

            nlohmann::json jdata = m.data.at(0);
            for (auto& [key, value] : jdata.items())
            {
                std::cout << key << ": " << value << std::endl;
            }

            std::cout << "Data inside data:" << std::endl;

            nlohmann::json jdataData = m.data.at(0).at("data");
            for (auto& [key, value] : jdataData.items())
            {
                std::cout << key << ": " << value << std::endl;
            }

            // change status and push again
            if (jdataData.at("status") == "Pending")
            {
                std::cout << "Message status is Pending. Updating status and pushing back." << std::endl;
                jdataData["status"] = "InProcess";
                Message newMessage(MessageType::COMMAND, jdataData, "CommandManager");
                // m_agentQueue.push(newMessage);
                // return m;
            }

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
