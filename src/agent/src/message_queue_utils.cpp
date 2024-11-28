#include <imultitype_queue.hpp>
#include <message_queue_utils.hpp>

#include <vector>

boost::asio::awaitable<std::tuple<int, std::string>>
GetMessagesFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue,
                     MessageType messageType,
                     int numMessages,
                     std::function<std::string()> getMetadataInfo)
{
    const auto messages = co_await multiTypeQueue->getNextNAwaitable(messageType, numMessages, "", "");

    std::string output;

    if (getMetadataInfo != nullptr)
    {
        output = getMetadataInfo();
    }

    for (const auto& message : messages)
    {
        output += "\n" + message.metaData + "\n" + message.data.dump();
    }

    co_return std::tuple<int, std::string> {messages.size(), output};
}

void PopMessagesFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue, MessageType messageType, int numMessages)
{
    multiTypeQueue->popN(messageType, numMessages);
}

void PushCommandsToQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue, const std::string& commands)
{
    const auto jsonObj = nlohmann::json::parse(commands);

    if (jsonObj.contains("commands") && jsonObj["commands"].is_array())
    {
        std::vector<Message> messages;

        for (const auto& command : jsonObj["commands"])
        {
            messages.emplace_back(MessageType::COMMAND, command);
        }

        if (!messages.empty())
        {
            multiTypeQueue->push(messages);
        }
    }
}

std::optional<module_command::CommandEntry> GetCommandFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue)
{
    if (multiTypeQueue->isEmpty(MessageType::COMMAND))
    {
        return std::nullopt;
    }

    Message m = multiTypeQueue->getNext(MessageType::COMMAND);
    nlohmann::json jsonData = m.data;

    std::string id;
    std::string module;
    std::string command;
    nlohmann::json parameters = nlohmann::json::array();

    if (jsonData.contains("id") && jsonData["id"].is_string())
    {
        id = jsonData["id"].get<std::string>();
    }

    if (jsonData.contains("args") && jsonData["args"].is_array())
    {
        int index = 0;
        for (const auto& arg : jsonData["args"])
        {
            switch (index++)
            {
                case 0:
                    if (arg.is_string())
                        module = arg.get<std::string>();
                    break;
                case 1:
                    if (arg.is_string())
                        command = arg.get<std::string>();
                    break;
                default: parameters.push_back(arg); break;
            }
        }
    }

    module_command::CommandEntry cmd(id, module, command, parameters, "", module_command::Status::IN_PROGRESS);

    return cmd;
}

void PopCommandFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue)
{
    multiTypeQueue->pop(MessageType::COMMAND);
}
