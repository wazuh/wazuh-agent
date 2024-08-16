#include <agent.hpp>

#include <http_client.hpp>
#include <message.hpp>
#include <message_queue_utils.hpp>
#include <signal_handler.hpp>

#include <memory>
#include <string>
#include <thread>

Agent::Agent(std::unique_ptr<ISignalHandler> signalHandler)
    : m_messageQueue(std::make_shared<MultiTypeQueue>())
    , m_signalHandler(std::move(signalHandler))
    , m_communicator(std::make_unique<http_client::HttpClient>(),
                     m_agentInfo.GetUUID(),
                     m_agentInfo.GetKey(),
                     [this](std::string table, std::string key) -> std::string
                     { return m_configurationParser.GetConfig<std::string>(std::move(table), std::move(key)); })
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

    const nlohmann::json dataContent = {{"command", "upgradeModule"}, {"status", "Pending"}};
    const Message messageToSend {MessageType::COMMAND, dataContent, "CommandHandler"};
    m_agentQueue.push(messageToSend);

    m_taskManager.EnqueueTask(m_communicator.WaitForTokenExpirationAndAuthenticate());

    m_taskManager.EnqueueTask(m_communicator.GetCommandsFromManager(
        [this](const std::string& response) { PushCommandsToQueue(m_messageQueue, response); }));

    m_taskManager.EnqueueTask(m_communicator.StatefulMessageProcessingTask(
        [this]() { return GetMessagesFromQueue(m_messageQueue, MessageType::STATEFUL); },
        [this]([[maybe_unused]] const std::string& response)
        { PopMessagesFromQueue(m_messageQueue, MessageType::STATEFUL); }));

    m_taskManager.EnqueueTask(m_communicator.StatelessMessageProcessingTask(
        [this]() { return GetMessagesFromQueue(m_messageQueue, MessageType::STATELESS); },
        [this]([[maybe_unused]] const std::string& response)
        { PopMessagesFromQueue(m_messageQueue, MessageType::STATELESS); }));

    m_taskManager.EnqueueTask(m_commandHandler.ProcessCommandsFromQueue<Message>(
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
            // if (jdataData.at("status") == "Pending")
            //{
            //     std::cout << "Message status is Pending. Updating status and pushing back." << std::endl;
            //     jdataData["status"] = "InProcess";
            //     Message newMessage(MessageType::COMMAND, jdataData, "CommandHandler");
            //     // m_agentQueue.push(newMessage);
            //     // return m;
            // }

            return m;
        },
        [this](Message& cmd) -> int
        {
            std::cout << "Dispatching command " << cmd.moduleName << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return 1;
        }));

    m_signalHandler->WaitForSignal();
    m_communicator.Stop();
    m_commandHandler.Stop();
}
