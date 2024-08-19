#include <message_queue_utils.hpp>

#include <imultitype_queue.hpp>

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

std::string findJsonKey(const nlohmann::json& j, const std::string& key)
{
    if (j.is_object())
    {
        for (const auto& [k, v] : j.items())
        {
            if (k == key)
            {
                return v.get<std::string>();
            }
            else if (v.is_object() || v.is_array())
            {
                std::string result = findJsonKey(v, key);
                if (!result.empty())
                {
                    return result;
                }
            }
        }
    }
    else if (j.is_array())
    {
        for (const auto& element : j)
        {
            std::string result = findJsonKey(element, key);
            if (!result.empty())
            {
                return result;
            }
        }
    }

    return ""; // key not found
}

bool setJsonValue(nlohmann::json& j, const std::string& key, const std::string& value)
{

    if (j.is_object())
    {
        for (auto& [k, v] : j.items())
        {
            if (k == key)
            {
                v = value;
                return true;
            }
            else if (v.is_object() || v.is_array())
            {
                if (setJsonValue(v, key, value))
                {
                    return true;
                }
            }
        }
    }
    else if (j.is_array())
    {
        for (auto& element : j)
        {
            if (setJsonValue(element, key, value))
            {
                return true;
            }
        }
    }

    return false; // Key not found
}
