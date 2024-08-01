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
        // FIXME
        //  else
        //  {
        //      std::cout << "Can failed because os full queue" << std::endl;
        //  }

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
                        m_persistenceDest->Store(singleMessageData, sMessageType);
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
                m_persistenceDest->Store(message.data, m_mapMessageTypeName.at(message.type));
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

Message MultiTypeQueue::getLastMessage(MessageType type)
{
    Message result(type, {});
    // std::unique_lock<std::mutex> mapLock(m_mapMutex);
    if (m_mapMessageTypeName.contains(type))
    {
        result.data = m_persistenceDest->RetrieveMultiple(1, m_mapMessageTypeName.at(type));
    }
    else
    {
        // TODO: error / logging handling !!!
        std::cout << "error didn't find the queue" << std::endl;
    }
    return result;
}

Message MultiTypeQueue::getNMessages(MessageType type, int messageQuantity)
{
    Message result(type, {});
    if (m_mapMessageTypeName.contains(type))
    {
        result.data = m_persistenceDest->RetrieveMultiple(messageQuantity, m_mapMessageTypeName.at(type));
    }
    else
    {
        // TODO: error / logging handling !!!
        std::cout << "error didn't find the queue" << std::endl;
    }
    return result;
}

bool MultiTypeQueue::popLastMessage(MessageType type)
{
    bool result = false;
    if (m_mapMessageTypeName.contains(type))
    {
        // Handle return value
        result = m_persistenceDest->RemoveMultiple(1, m_mapMessageTypeName.at(type));
    }
    else
    {
        // TODO: error / logging handling !!!
        std::cout << "error didn't find the queue" << std::endl;
    }
    return result;
}

bool MultiTypeQueue::popNMessages(MessageType type, int messageQuantity)
{
    bool result = false;
    if (m_mapMessageTypeName.contains(type))
    {
        result = m_persistenceDest->RemoveMultiple(messageQuantity, m_mapMessageTypeName.at(type));
    }
    else
    {
        // TODO: error / logging handling !!!
        std::cout << "error didn't find the queue" << std::endl;
    }
    return result;
}

bool MultiTypeQueue::isEmptyByType(MessageType type)
{
    if (m_mapMessageTypeName.contains(type))
    {
        return m_persistenceDest->GetElementCount(m_mapMessageTypeName.at(type)) == 0;
    }
    else
    {
        // TODO: error / logging handling !!!
        std::cout << "error didn't find the queue" << std::endl;
    }
    return false;
}

bool MultiTypeQueue::isFullByType(MessageType type)
{
    if (m_mapMessageTypeName.contains(type))
    {
        return m_persistenceDest->GetElementCount(m_mapMessageTypeName.at(type)) == m_maxItems;
    }
    else
    {
        // TODO: error / logging handling !!!
        std::cout << "error didn't find the queue" << std::endl;
    }
    return false;
}

int MultiTypeQueue::getItemsByType(MessageType type)
{
    if (m_mapMessageTypeName.contains(type))
    {
        return m_persistenceDest->GetElementCount(m_mapMessageTypeName.at(type));
    }
    else
    {
        // TODO: error / logging handling !!!
        std::cout << "error didn't find the queue" << std::endl;
    }
    return false;
}
