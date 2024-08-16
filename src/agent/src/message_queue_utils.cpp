#include <message_queue_utils.hpp>

#include <imultitype_queue.hpp>

#include <nlohmann/json.hpp>

#include <vector>

namespace
{
    // This should eventually be replaced with a configuration parameter.
    constexpr int NUM_EVENTS = 1;
} // namespace

boost::asio::awaitable<std::string> getMessagesFromQueue(IMultiTypeQueue& multiTypeQueue, MessageType messageType)
{
    const auto message = co_await multiTypeQueue.getNextNAwaitable(messageType, NUM_EVENTS);

    nlohmann::json jsonObj;
    jsonObj["events"] = nlohmann::json::array();
    jsonObj["events"].push_back(message.data);

    co_return jsonObj.dump();
}

void popMessagesFromQueue(IMultiTypeQueue& multiTypeQueue, MessageType messageType)
{
    multiTypeQueue.popN(messageType, NUM_EVENTS);
}

void pushCommandsToQueue(IMultiTypeQueue& multiTypeQueue, const std::string& commands)
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
            multiTypeQueue.push(messages);
        }
    }
}
