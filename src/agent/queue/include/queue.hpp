#ifndef QUEUE_H
#define QUEUE_H

#include <any>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <vector>

#include "shared.hpp"
#include "sqlitestorage.h"

// TODO: move to a configuration setting
constexpr int DEFAULT_MAX = 10;
constexpr int DEFAULT_TIMEOUT_S = 3;

/**
 * @brief Map for transforing Message type name to string
 *
 */
static const std::map<MessageType, std::string> MessageTypeName {
    {MessageType::STATELESS, "STATELESS"},
    {MessageType::STATEFUL, "STATEFUL"},
    {MessageType::COMMAND, "COMMAND"},
};

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
private:
    const MessageType m_queueType;
    const int m_max_size;
    const std::chrono::seconds m_timeout;
    std::unique_ptr<Persistence> m_persistenceDest;
    std::mutex m_mtx;
    std::condition_variable m_cv;

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
    int getItemsAvailable();

    /**
     * @brief
     *
     * @param event
     * @return true
     * @return false
     */
    bool insertMessage(Message event);

    /**
     * @brief
     *
     * @param qttyMessages
     * @return true
     * @return false
     */
    bool removeNMessages(int qttyMessages);

    /**
     * @brief Get the Message object
     *
     * @return Message
     */
    Message getMessage();

    /**
     * @brief
     *
     * @param n
     * @return Message
     */
    Message getNMessages(int n);

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool empty();

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isFull();

    void waitUntilNotFull();

    /**
     * A function that waits until the queue is not full or until a stop signal is received.
     *
     * @param stopToken std::stop_token to check for a stop signal
     *
     * @throws None
     */
    void waitUntilNotFullOrStoped(std::stop_token stopToken);
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

    /**
     * @brief: timeoutPush message to a queue of t
     *
     * @param message
     */
    bool timeoutPush(Message message, bool shouldWait = false);

    /**
     * @brief
     *
     * @param messages
     */
    void timeoutPush(std::vector<Message> messages);

    /**
     * @brief Get the Last Message object
     *
     * @param type
     * @return Message
     */
    Message getLastMessage(MessageType type);

    /**
     * @brief deletes a message from a queue
     *
     * @param type MessageType queue to pop
     * @return true popped succesfully
     * @return false wasn't able to pop message
     */
    bool popLastMessage(MessageType type);

    /**
     * @brief
     *
     * @param type
     * @param messageQuantity
     * @return true
     * @return false
     */
    bool popNMessages(MessageType type, int messageQuantity);

    /**
     * @brief
     *
     * @param type
     * @return true
     * @return false
     */
    bool isEmptyByType(MessageType type);

    /**
     * @brief Get the Items By Type object
     *
     * @param type
     * @return true
     * @return false
     */
    int getItemsByType(MessageType type);
};

#endif // QUEUE_H
