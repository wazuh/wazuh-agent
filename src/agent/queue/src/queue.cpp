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

int PersistedQueue::getItemsAvailable()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    return m_persistenceDest->GetElementCount();
}

bool PersistedQueue::insertMessage(Message event)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    bool success = false;
    size_t spaceAvailable =
        (m_max_size > m_persistenceDest->GetElementCount()) ? m_max_size - m_persistenceDest->GetElementCount() : 0;
    if (spaceAvailable)
    {
        // TODO: handle response
        success = true;
        auto messageData = event.data;
        if (messageData.is_array())
        {
            if (messageData.size() <= spaceAvailable)
            {
                for (const auto& singleMessageData : messageData)
                {
                    m_persistenceDest->Store(singleMessageData);
                    m_cv.notify_all();
                }
            }
            else
            {
                success = false;
            }
        }
        else
        {
            m_persistenceDest->Store(event.data);
            m_cv.notify_all();
        }
    }
    return success;
};

bool PersistedQueue::removeNMessages(int qttyMessages)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    auto linesRemoved = m_persistenceDest->RemoveMultiple(qttyMessages);
    m_cv.notify_all();
    return linesRemoved != 0;
};

Message PersistedQueue::getMessage()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    auto messageData = m_persistenceDest->RetrieveMultiple(1);
    return Message(m_queueType, messageData);
};

Message PersistedQueue::getNMessages(int n)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    auto messageData = m_persistenceDest->RetrieveMultiple(n);
    return Message(m_queueType, messageData);
};

bool PersistedQueue::empty()
{
    const auto items = getItemsAvailable();
    return items == 0;
}

bool PersistedQueue::isFull()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    return m_persistenceDest->GetElementCount() == m_max_size;
}

void PersistedQueue::waitUntilNotFull()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cv.wait_for(lock, m_timeout, [this] { return m_persistenceDest->GetElementCount() < m_max_size; });
}

void PersistedQueue::waitUntilNotFullOrStoped(std::stop_token stopToken)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cv.wait(lock,
              [this, stopToken]
              {
                  bool menor = (m_persistenceDest->GetElementCount() < m_max_size);
                  bool stopped = (stopToken.stop_possible() && stopToken.stop_requested());
                  return menor || stopped;
              });
}

bool MultiTypeQueue::timeoutPush(Message message, bool shouldWait)
{
    bool result = false;
    std::unique_lock<std::mutex> mapLock(m_mapMutex);

    if (m_queuesMap.contains(message.type))
    {
        auto& queue = m_queuesMap[message.type];
        mapLock.unlock();

        // Wait until the queue is not full
        if (shouldWait)
        {
            queue->waitUntilNotFull();
        }
        // FIXME
        //  else
        //  {
        //      std::cout << "Can failed because os full queue" << std::endl;
        //  }

        // Insert the message
        result = queue->insertMessage(message);
        if (!result)
        {
            std::cout << "Failed to insert message: " << message.data << std::endl;
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
    std::unique_lock<std::mutex> mapLock(m_mapMutex);
    if (m_queuesMap.contains(type))
    {
        auto& queue = m_queuesMap[type];
        mapLock.unlock();
        result = queue->getMessage();
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
    std::unique_lock<std::mutex> mapLock(m_mapMutex);
    if (m_queuesMap.contains(type))
    {
        auto& queue = m_queuesMap[type];
        mapLock.unlock();
        // Handle return value
        result = queue->removeNMessages(1);
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
    std::unique_lock<std::mutex> mapLock(m_mapMutex);
    if (m_queuesMap.contains(type))
    {
        auto& queue = m_queuesMap[type];
        mapLock.unlock();
        result = queue->removeNMessages(messageQuantity);
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
    std::unique_lock<std::mutex> mapLock(m_mapMutex);
    if (m_queuesMap.contains(type))
    {
        auto& queue = m_queuesMap[type];
        mapLock.unlock();
        return queue->empty();
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
    std::unique_lock<std::mutex> mapLock(m_mapMutex);
    if (m_queuesMap.contains(type))
    {
        auto& queue = m_queuesMap[type];
        mapLock.unlock();
        return queue->getItemsAvailable();
    }
    else
    {
        // TODO: error / logging handling !!!
        std::cout << "error didn't find the queue" << std::endl;
    }
    return false;
}
