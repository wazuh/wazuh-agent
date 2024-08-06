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
    Message getLastMessage(MessageType type, const std::string module = "");

    /**
     * @brief Returns N messages from a queue
     *
     * @param type Of the queue to be used as source
     * @param messageQuantity quantity of messages to return
     * @return Message Json data othe messages fetched
     */
    Message getNMessages(MessageType type, int messageQuantity, const std::string moduleName = "");

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
     * @brief Checks emptyness of a queue
     *
     * @param type
     * @return true when queue empty
     * @return false otherwise
     */
    bool isEmptyByType(MessageType type);

    /**
     * @brief Checks fullness of a queue
     *
     * @param type
     * @return true when queue is full
     * @return false otherwise
     */
    bool isFullByType(MessageType type);

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
