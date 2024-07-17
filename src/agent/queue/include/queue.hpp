#include <any>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

#include "filestorage.hpp"
#include "shared.hpp"
#include "sqlitestorage.h"

// TODO: move to a configuration setting
constexpr int DEFAULT_MAX = 10;

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
        else if (type == "File")
        {
            if (args.size() != 1 || !std::any_cast<std::string>(&args[0]))
            {
                throw std::invalid_argument("File requires a string argument");
            }
            return std::make_unique<FileStorage>(std::any_cast<std::string>(args[0]));
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
    PersistedQueue(MessageType m_queueType, int max_size = DEFAULT_MAX)
        : m_queueType(m_queueType)
        , m_max_size(max_size)
    {
        // Another option:
        // m_persistenceDest = PersistenceFactory::createPersistence("File", {DEFAULT_FILE_PATH +
        // MessageTypeName.at(m_queueType)});
        m_persistenceDest = PersistenceFactory::createPersistence(
            "SQLite3", {static_cast<std::string>(DEFAULT_DB_PATH), MessageTypeName.at(m_queueType)});
        // updates the actual size being used
        getItemsAvailable();
    }

    // Delete copy constructor
    PersistedQueue(const PersistedQueue&) = delete;

    // Delete copy assignment operator
    PersistedQueue& operator=(const PersistedQueue&) = delete;

    // Delete move constructor
    PersistedQueue(PersistedQueue&&) = delete;

    // Delete move assignment operator
    PersistedQueue& operator=(PersistedQueue&&) = delete;

    // TODO
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

    int getItemsAvailable()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        //TODO: rework this behavior
        m_size = m_persistenceDest->GetElementCount();
        return m_size;
    }

    /**
     * @brief Get the Size object
     *
     * @return int
     */
    // TODO check this functionality because when inserting data arrays it loses actual value
    int getSize() const
    {
        return m_size;
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
        m_persistenceDest->Store(event.data);
        m_size++;
        m_cv.notify_one();
        // TODO: make Store method int for returning items inserted when array
        // if(m_persistenceDest->Store(event.data))
        // {
        //     m_size++;
        //     m_cv.notify_one();
        // }
        return true;
    };

    bool removeNMessages(int qttyMessages)
    {
        bool result = false;
        // workaround for items issue
        getItemsAvailable();
        if (m_size)
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            auto linesRemoved = m_persistenceDest->RemoveMultiple(qttyMessages);
            if(linesRemoved)
            {
                result = true;
                m_size = m_size - linesRemoved;
            }
        }
        return result;
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
    bool empty() const
    {
        return m_size == 0;
    }

private:
    const MessageType m_queueType;
    std::atomic<int> m_size = 0;
    const int m_max_size;
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

public:
    // Create a vector with 3 PersistedQueue elements
    MultiTypeQueue(int size = DEFAULT_MAX)
        : m_maxItems(size)
    {
        // Populate the map inside the constructor body
        m_queuesMap[MessageType::STATELESS] = std::make_unique<PersistedQueue>(MessageType::STATELESS, m_maxItems);
        m_queuesMap[MessageType::STATEFUL] = std::make_unique<PersistedQueue>(MessageType::STATEFUL, m_maxItems);
        m_queuesMap[MessageType::COMMAND] = std::make_unique<PersistedQueue>(MessageType::COMMAND, m_maxItems);
    }

    // Delete copy constructor
    MultiTypeQueue(const MultiTypeQueue&) = delete;

    // Delete copy assignment operator
    MultiTypeQueue& operator=(const MultiTypeQueue&) = delete;

    // Delete move constructor
    MultiTypeQueue(MultiTypeQueue&&) = delete;

    // Delete move assignment operator
    MultiTypeQueue& operator=(MultiTypeQueue&&) = delete;

    // TODO
    ~MultiTypeQueue() {};
    /**
     * @brief: push message to a queue of t
     *
     * @param message
     */
    bool push(Message message)
    {
        bool result = false;
        if (m_queuesMap.contains(message.type))
        {
            while (m_queuesMap[message.type]->getSize() == m_maxItems)
            {
                // TODO: delete this
                std::cout << "waiting" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }
            result = m_queuesMap[message.type]->insertMessage(message);
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
     * @param messages 
     */
    void push(std::vector<Message> messages)
    {
        for (const auto& singleMessage : messages)
        {
            push(singleMessage);
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
        if (m_queuesMap.contains(type))
        {
            result = m_queuesMap[type]->getMessage();
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
        if (m_queuesMap.contains(type))
        {
            // Handle return value
            result = m_queuesMap[type]->removeNMessages(1);
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
        if (m_queuesMap.contains(type))
        {
            result = m_queuesMap[type]->removeNMessages(messageQuantity);
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
        if (m_queuesMap.contains(type))
        {
            return m_queuesMap[type]->empty();
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
        if (m_queuesMap.contains(type))
        {
            return m_queuesMap[type]->getItemsAvailable();
        }
        else
        {
            // TODO: error / logging handling !!!
            std::cout << "error didn't find the queue" << std::endl;
        }
        return false;
    }
};
