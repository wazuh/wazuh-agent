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

#include "shared.hpp"
#include "sqlitestorage.h"

// TODO: move to a configuration setting
constexpr int DEFAULT_MAX = 10;
constexpr int DEFAULT_TIMEOUT_S = 3;

// Factory class
class PersistenceFactory
{
public:
    static std::unique_ptr<Persistence> createPersistence(const std::string& type, const std::vector<std::any>& args)
    {
        if (type == "SQLite3")
        {
            if (args.size() != 2 || !std::any_cast<std::string>(&args[0]) || !std::any_cast<std::string>(&args[1]))
            {
                throw std::invalid_argument("SQLite3 requires 2  string arguments");
            }
            return std::make_unique<SQLiteStorage>(std::any_cast<std::string>(args[0]),
                                                   std::any_cast<std::string>(args[1]));
        }
        throw std::runtime_error("Unknown persistence type");
    }
};

/**
 * @brief Class representing each single type queue
 *
 */
class PersistedQueue
{
public:
    PersistedQueue(MessageType queueType, int max_size, int timeout)
        : m_queueType(queueType)
        , m_max_size(max_size)
        , m_timeout(timeout)
    {
        m_persistenceDest = PersistenceFactory::createPersistence(
            "SQLite3", {static_cast<std::string>(DEFAULT_DB_PATH), MessageTypeName.at(m_queueType)});
    }

    // Delete copy constructor
    PersistedQueue(const PersistedQueue&) = delete;

    // Delete copy assignment operator
    PersistedQueue& operator=(const PersistedQueue&) = delete;

    // Delete move constructor
    PersistedQueue(PersistedQueue&&) = delete;

    // Delete move assignment operator
    PersistedQueue& operator=(PersistedQueue&&) = delete;

    ~PersistedQueue() {
        // m_persistenceDest.close();
    };

    /**
     * @brief Get the Type object
     *
     * @return const MessageType&
     */
    const MessageType& getType() const
    {
        return m_queueType;
    }

    /**
     * @brief Get the Items Available object
     *
     * @return int
     */
    int getItemsAvailable()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        return m_persistenceDest->GetElementCount();
    }

    /**
     * @brief
     *
     * @param event
     * @return true
     * @return false
     */
    bool insertMessage(Message event)
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

    /**
     * @brief
     *
     * @param qttyMessages
     * @return true
     * @return false
     */
    bool removeNMessages(int qttyMessages)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        auto linesRemoved = m_persistenceDest->RemoveMultiple(qttyMessages);
        m_cv.notify_all();
        return linesRemoved != 0;
    };

    /**
     * @brief Get the Message object
     *
     * @return Message
     */
    Message getMessage()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        auto messageData = m_persistenceDest->RetrieveMultiple(1);
        return Message(m_queueType, messageData);
    };

    /**
     * @brief
     *
     * @param n
     * @return Message
     */
    Message getNMessages(int n)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        auto messageData = m_persistenceDest->RetrieveMultiple(n);
        return Message(m_queueType, messageData);
    };

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool empty()
    {
        const auto items = getItemsAvailable();
        return items == 0;
    }

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isFull()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        return m_persistenceDest->GetElementCount() == m_max_size;
    }

    void waitUntilNotFull()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_cv.wait_for(lock,
                      m_timeout,
                      [this]
                      {
                          std::cout << " waiting " << std::endl;
                          return m_persistenceDest->GetElementCount() < m_max_size;
                      });
    }

    /**
     * A function that waits until the queue is not full or until a stop signal is received.
     *
     * @param stopToken std::stop_token to check for a stop signal
     *
     * @throws None
     */
    void waitUntilNotFullOrStoped(std::stop_token stopToken)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_cv.wait(lock,
                  [this, stopToken]
                  {
                      std::cout << " waiting " << std::endl;
                      bool menor = (m_persistenceDest->GetElementCount() < m_max_size);
                      bool stopped = (stopToken.stop_possible() && stopToken.stop_requested());
                      std::cout << "menor" << menor << "stopped" <<  stopped << std::endl;
                      return  menor || stopped ;
                  });
    }

private:
    const MessageType m_queueType;
    const int m_max_size;
    const std::chrono::seconds m_timeout;
    std::unique_ptr<Persistence> m_persistenceDest;
    std::mutex m_mtx;
    std::condition_variable m_cv;
};

/**
 * @brief
 *
 */
class MultiTypeQueue
{
private:
    std::unordered_map<MessageType, std::unique_ptr<PersistedQueue>> m_queuesMap;
    const int m_maxItems;
    std::mutex m_mapMutex;

public:
    // Create a vector with 3 PersistedQueue elements
    MultiTypeQueue(int size = DEFAULT_MAX, int timeout = DEFAULT_TIMEOUT_S)
        : m_maxItems(size)
    {
        // Populate the map inside the constructor body
        m_queuesMap[MessageType::STATELESS] =
            std::make_unique<PersistedQueue>(MessageType::STATELESS, m_maxItems, timeout);
        m_queuesMap[MessageType::STATEFUL] =
            std::make_unique<PersistedQueue>(MessageType::STATEFUL, m_maxItems, timeout);
        m_queuesMap[MessageType::COMMAND] = std::make_unique<PersistedQueue>(MessageType::COMMAND, m_maxItems, timeout);
    }

    // Delete copy constructor
    MultiTypeQueue(const MultiTypeQueue&) = delete;

    // Delete copy assignment operator
    MultiTypeQueue& operator=(const MultiTypeQueue&) = delete;

    // Delete move constructor
    MultiTypeQueue(MultiTypeQueue&&) = delete;

    // Delete move assignment operator
    MultiTypeQueue& operator=(MultiTypeQueue&&) = delete;

    ~MultiTypeQueue() {};

    bool stopablePush(Message message, std::stop_token stopToken)
    {
        bool result = false;
        std::unique_lock<std::mutex> mapLock(m_mapMutex);

        if (m_queuesMap.contains(message.type))
        {
            auto& queue = m_queuesMap[message.type];
            mapLock.unlock();

            // Wait until the queue is not full
            if (stopToken.stop_possible())
            {
                queue->waitUntilNotFullOrStoped(stopToken);
            }

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

    /**
     * @brief: timeoutPush message to a queue of t
     *
     * @param message
     */
    bool timeoutPush(Message message, bool shouldWait = false)
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
            //FIXME
            // else
            // {
            //     std::cout << "Can failed because os full queue" << std::endl;
            // }

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

    /**
     * @brief
     *
     * @param messages
     */
    void timeoutPush(std::vector<Message> messages)
    {
        for (const auto& singleMessage : messages)
        {
            timeoutPush(singleMessage);
        }
    }

    /**
     * @brief Get the Last Message object
     *
     * @param type
     * @return Message
     */
    Message getLastMessage(MessageType type)
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

    /**
     * @brief deletes a message from a queue
     *
     * @param type MessageType queue to pop
     * @return true popped succesfully
     * @return false wasn't able to pop message
     */
    bool popLastMessage(MessageType type)
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

    /**
     * @brief
     *
     * @param type
     * @param messageQuantity
     * @return true
     * @return false
     */
    bool popNMessages(MessageType type, int messageQuantity)
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

    /**
     * @brief
     *
     * @param type
     * @return true
     * @return false
     */
    bool isEmptyByType(MessageType type)
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

    /**
     * @brief Get the Items By Type object
     *
     * @param type
     * @return true
     * @return false
     */
    int getItemsByType(MessageType type)
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
};
