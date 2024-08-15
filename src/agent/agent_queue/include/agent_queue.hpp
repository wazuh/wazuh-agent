#ifndef QUEUE_H
#define QUEUE_H

#include <any>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <vector>

#include <boost/asio.hpp>
#include <cassert>

#include "persistence_factory.hpp"
#include "shared.hpp"
#include "sqlitestorage.h"

// TODO: move to a configuration setting
constexpr int DEFAULT_MAX = 10000;
constexpr int DEFAULT_TIMEOUT_S = 3;

class MultiTypeQueue
{
private:
    const std::vector<std::string> m_vMessageTypeStrings {"STATELESS", "STATEFUL", "COMMAND"};
    const std::map<MessageType, std::string> m_mapMessageTypeName {
        {MessageType::STATELESS, "STATELESS"},
        {MessageType::STATEFUL, "STATEFUL"},
        {MessageType::COMMAND, "COMMAND"},
    };
    const int m_maxItems;
    const std::chrono::seconds m_timeout;
    std::unique_ptr<Persistence> m_persistenceDest;
    std::mutex m_mtx;
    std::condition_variable m_cv;

public:
    MultiTypeQueue(int size = DEFAULT_MAX, int timeout = DEFAULT_TIMEOUT_S)
        : m_maxItems(size)
        , m_timeout(timeout)
    {
        try
        {
            m_persistenceDest = PersistenceFactory::createPersistence(
                "SQLite3", {static_cast<std::string>(DEFAULT_DB_PATH), m_vMessageTypeStrings});
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error creating persistence: " << e.what() << '\n';
        }
    }

    /**
     * @brief Delete copy constructor
     */
    MultiTypeQueue(const MultiTypeQueue&) = delete;

    /**
     * @brief Delete copy assignment operator
     */
    MultiTypeQueue& operator=(const MultiTypeQueue&) = delete;

    /**
     * @brief Delete move constructor
     */
    MultiTypeQueue(MultiTypeQueue&&) = delete;

    /**
     * @brief Delete move assignment operator
     */
    MultiTypeQueue& operator=(MultiTypeQueue&&) = delete;

    /**
     * @brief Destructor.
     */
    ~MultiTypeQueue() {};

    /**
     * @brief pushes a message
     *
     * @param message to be pushed
     * @param shouldWait when true, the function will wait until the message is pushed
     * @return int number of messages pushed
     */
    int push(Message message, bool shouldWait = false);

    /**
     * @brief pushes a message
     *
     * @param message to be pushed
     * @return boost::asio::awaitable<int> number of messages pushed
     */
    boost::asio::awaitable<int> pushAwaitable(Message message);

    /**
     * @brief pushes a vector of messages
     *
     * @param messages vector of messages to be pushed
     * @return int number of messages pushed
     */
    int push(std::vector<Message> messages);

    /**
     * @brief Get the next Message object
     *
     * @param type of the queue to be used as source
     * @param module module name
     * @return Message type object taken from the queue
     */
    Message getNext(MessageType type, const std::string module = "");

    /**
     * @brief Get the Next Awaitable object
     *
     * @param type of the queue to be used as source
     * @param moduleName module name
     * @param messageQuantity quantity of messages to return
     * @return boost::asio::awaitable<Message> awaitable object taken from the queue
     */
    boost::asio::awaitable<Message>
    getNextNAwaitable(MessageType type, int messageQuantity, const std::string moduleName = "");

    /**
     * @brief Returns N messages from a queue
     *
     * @param type Of the queue to be used as source
     * @param moduleName module name
     * @param messageQuantity quantity of messages to return
     * @return Message Json data othe messages fetched
     */
    std::vector<Message> getNextN(MessageType type, int messageQuantity, const std::string moduleName = "");

    /**
     * @brief deletes a message from a queue
     *
     * @param type MessageType queue to pop
     * @param moduleName
     * @return true when popped succesfully
     * @return false if it wasn't able to pop message
     */
    bool pop(MessageType type, const std::string moduleName = "");

    /**
     * @brief deletes N messages from a queue
     *
     * @param type MessageType queue to pop
     * @param moduleName module name
     * @param messageQuantity quantity of messages to pop
     * @return Number of messages deleted
     */
    int popN(MessageType type, int messageQuantity, const std::string moduleName = "");

    /**
     * @brief Checks emptyness of a queue
     *
     * @param type MessageType
     * @param moduleName module name
     * @return true when queue empty
     * @return false otherwise
     */
    bool isEmpty(MessageType type, const std::string moduleName = "");

    /**
     * @brief Checks fullness of a queue
     *
     * @param type MessageType
     * @param moduleName module name
     * @return true when queue is full
     * @return false otherwise
     */
    bool isFull(MessageType type, const std::string moduleName = "");

    /**
     * @brief Get the Items By Type object
     *
     * @param type MessageType
     * @param moduleName module name
     * @return int number of items in the queue.
     */
    int storedItems(MessageType type, const std::string moduleName = "");
};

#endif // QUEUE_H
