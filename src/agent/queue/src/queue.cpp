#include <any>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <stop_token>
#include <unordered_map>
#include <utility>
#include <vector>

#include "queue.hpp"

bool MultiTypeQueue::timeoutPush(Message message, bool shouldWait)
{
    bool result = false;

    if (m_mapMessageTypeName.contains(message.type))
    {
        auto sMessageType = m_mapMessageTypeName.at(message.type);

        // Wait until the queue is not full
        if (shouldWait)
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.wait_for(
                lock, m_timeout, [&, this] { return m_persistenceDest->GetElementCount(sMessageType) < m_maxItems; });
        }

        auto storedMessages = m_persistenceDest->GetElementCount(sMessageType);
        size_t spaceAvailable = (m_maxItems > storedMessages) ? m_maxItems - storedMessages : 0;
        if (spaceAvailable)
        {
            // TODO: handle response
            result = true;
            auto messageData = message.data;
            if (messageData.is_array())
            {
                if (messageData.size() <= spaceAvailable)
                {
                    for (const auto& singleMessageData : messageData)
                    {
                        m_persistenceDest->Store(singleMessageData, sMessageType, message.moduleName);
                        m_cv.notify_all();
                    }
                }
                else
                {
                    result = false;
                }
            }
            else
            {
                m_persistenceDest->Store(message.data, m_mapMessageTypeName.at(message.type), message.moduleName);
                m_cv.notify_all();
            }
        }
    }
    else
    {
        std::cout << "error didn't find the queue" << std::endl;
    }
    return result;
}

void MultiTypeQueue::timeoutPush(std::vector<Message> messages)
{
    for (const auto& singleMessage : messages)
    {
        timeoutPush(singleMessage);
    }
}

Message MultiTypeQueue::getLastMessage(MessageType type, const std::string moduleName)
{
    // TODO: If moduleName is not emty should be part of result on the pther cases should be parsed from answer
    Message result(type, "{}"_json, moduleName);
    if (m_mapMessageTypeName.contains(type))
    {
        auto resultData = m_persistenceDest->RetrieveMultiple(1, m_mapMessageTypeName.at(type), moduleName);
        if (!resultData.empty())
        {
            result.data = resultData;
        }
    }
    else
    {
        // TODO: error handling and logging
        std::cout << "error didn't find the queue" << std::endl;
    }
    return result;
}

std::vector<Message> MultiTypeQueue::getNMessages(MessageType type, int messageQuantity, const std::string moduleName)
{
    std::vector<Message> result;
    if (m_mapMessageTypeName.contains(type))
    {
        auto arrayData =
            m_persistenceDest->RetrieveMultiple(messageQuantity, m_mapMessageTypeName.at(type), moduleName);
        for (auto singleJson : arrayData)
        {
            auto finalModuleName = moduleName;
            if (moduleName.empty())
            {
                finalModuleName = singleJson["module"];
            }
            result.push_back(Message(type, singleJson, finalModuleName));
        }
    }
    else
    {
        // TODO: error handling and logging
        std::cout << "error didn't find the queue" << std::endl;
    }
    return result;
}

bool MultiTypeQueue::popLastMessage(MessageType type, const std::string moduleName)
{
    bool result = false;
    if (m_mapMessageTypeName.contains(type))
    {
        // TODO: Handle return value -> should show how many rows where deleted
        result = m_persistenceDest->RemoveMultiple(1, m_mapMessageTypeName.at(type), moduleName);
    }
    else
    {
        // TODO: error handling and logging
        std::cout << "error didn't find the queue" << std::endl;
    }
    return result;
}

bool MultiTypeQueue::popNMessages(MessageType type, int messageQuantity, const std::string moduleName)
{
    bool result = false;
    if (m_mapMessageTypeName.contains(type))
    {
        result = m_persistenceDest->RemoveMultiple(messageQuantity, m_mapMessageTypeName.at(type), moduleName);
    }
    else
    {
        // TODO: error handling and logging
        std::cout << "error didn't find the queue" << std::endl;
    }
    return result;
}

bool MultiTypeQueue::isEmptyByType(MessageType type, const std::string moduleName)
{
    if (m_mapMessageTypeName.contains(type))
    {
        return m_persistenceDest->GetElementCount(m_mapMessageTypeName.at(type), moduleName) == 0;
    }
    else
    {
        // TODO: error handling and logging
        std::cout << "error didn't find the queue" << std::endl;
    }
    return false;
}

bool MultiTypeQueue::isFullByType(MessageType type, const std::string moduleName)
{
    if (m_mapMessageTypeName.contains(type))
    {
        return m_persistenceDest->GetElementCount(m_mapMessageTypeName.at(type), moduleName) == m_maxItems;
    }
    else
    {
        // TODO: error handling and logging
        std::cout << "error didn't find the queue" << std::endl;
    }
    return false;
}

int MultiTypeQueue::getItemsByType(MessageType type, const std::string moduleName)
{
    if (m_mapMessageTypeName.contains(type))
    {
        return m_persistenceDest->GetElementCount(m_mapMessageTypeName.at(type), moduleName);
    }
    else
    {
        // TODO: error handling and logging
        std::cout << "error didn't find the queue" << std::endl;
    }
    return false;
}
