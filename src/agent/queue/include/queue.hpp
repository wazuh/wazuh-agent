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
// #include <boost/asio/experimental/awaitable_operators.hpp>
#include <cassert>

#include "shared.hpp"
#include "sqlitestorage.h"

// TODO: move to a configuration setting
constexpr int DEFAULT_MAX = 10000;
constexpr int DEFAULT_TIMEOUT_S = 3;

// Factory class
class PersistenceFactory
{
public:
    static std::unique_ptr<Persistence> createPersistence(const std::string& type, const std::vector<std::any>& args)
    {
        if (type == "SQLite3")
        {
            if (args.size() != 2 || !std::any_cast<std::string>(&args[0]) ||
                !std::any_cast<std::vector<std::string>>(&args[1]))
            {
                throw std::invalid_argument("SQLite3 requires db name and table names as arguments");
            }
            return std::make_unique<SQLiteStorage>(std::any_cast<std::string>(args[0]),
                                                   std::any_cast<std::vector<std::string>>(args[1]));
        }
        throw std::runtime_error("Unknown persistence type");
    }
};

/**
 * @brief
 *
 */
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
    // Create a vector with 3 PersistedQueue elements
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
     * @param messages to be pushed
     * @param shouldWait when true, the function will wait until the message is pushed
     * @return int number of messages pushed
     */
    int push(std::vector<Message> messages);

    /**
     * @brief Get the Last Message object
     *
     * @param type of the queue to be used as source
     * @return Message type object taken from the queue
     */
    Message getNext(MessageType type, const std::string module = "");

    /**
     * @brief Get the Next Awaitable object
     *
     * @param type of the queue to be used as source
     * @param module module name
     * @param messageQuantity quantity of messages to return
     * @return boost::asio::awaitable<Message>
     */
    boost::asio::awaitable<Message>
    getNextNAwaitable(MessageType type, int messageQuantity, const std::string moduleName = "");
    /**
     * @brief Returns N messages from a queue
     *
     * @param type Of the queue to be used as source
     * @param moduleName
     * @param messageQuantity quantity of messages to return
     * @return Message Json data othe messages fetched
     */
    std::vector<Message> getNextN(MessageType type, int messageQuantity, const std::string moduleName = "");

    /**
     * @brief deletes a message from a queue
     *
     * @param type MessageType queue to pop
     * @param moduleName
     * @return true popped succesfully
     * @return false wasn't able to pop message
     */
    bool pop(MessageType type, const std::string moduleName = "");

    /**
     * @brief
     *
     * @param type
     * @param moduleName
     * @param messageQuantity
     * @return Number of messages deleted
     */
    int popN(MessageType type, int messageQuantity, const std::string moduleName = "");

    /**
     * @brief Checks emptyness of a queue
     *
     * @param type
     * @param moduleName
     * @return true when queue empty
     * @return false otherwise
     */
    bool isEmpty(MessageType type, const std::string moduleName = "");

    /**
     * @brief Checks fullness of a queue
     *
     * @param type
     * @param moduleName
     * @return true when queue is full
     * @return false otherwise
     */
    bool isFull(MessageType type, const std::string moduleName = "");

    /**
     * @brief Get the Items By Type object
     *
     * @param type
     * @param moduleName
     * @return true
     * @return false
     */
    int storedItems(MessageType type, const std::string moduleName = "");
};

#endif // QUEUE_H
