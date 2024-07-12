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
        return m_persistenceDest->GetElementCount();
    }

    /**
     * @brief Get the Size object
     *
     * @return int
     */
    // TODO check this functionality
    int getSize() const
    {
        return m_size;
    }

    /**
     * @brief Set the Size object
     *
     * @param m_size
     */
    void setSize(int m_size)
    {
        this->m_size = m_size;
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
        return true;
    };

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool removeMessage()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        auto linesRemoved = m_persistenceDest->RemoveMultiple(1);
        m_size = m_size - linesRemoved;
        return true;
    };

    bool removeNMessages(int qttyMessages)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        auto linesRemoved = m_persistenceDest->RemoveMultiple(qttyMessages);
        m_size = m_size - linesRemoved;
        return true;
    };

    /**
     * @brief Get the Message object
     *
     * @param type
     * @return Message
     */
    Message getMessage(MessageType type)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        return Message(type, m_persistenceDest->RetrieveMultiple(1));
    };

    Message getNMessages(MessageType type, int n)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        return Message(type, m_persistenceDest->RetrieveMultiple(n));
    };

    bool empty()
    {
        return m_size == 0;
    }

private:
    MessageType m_queueType;
    int m_size = 0;
    int m_max_size;
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
    // std::condition_variable m_cv;
    std::unordered_map<MessageType, std::unique_ptr<PersistedQueue>> m_queuesMap;
    int m_maxItems;

public:
    // Create a vector with 3 PersistedQueue elements
    MultiTypeQueue(int size = DEFAULT_MAX)
        : m_maxItems(size)
    {
        // Populate the map inside the constructor body
        m_queuesMap[MessageType::STATE_LESS] = std::make_unique<PersistedQueue>(MessageType::STATE_LESS, m_maxItems);
        m_queuesMap[MessageType::STATE_FULL] = std::make_unique<PersistedQueue>(MessageType::STATE_FULL, m_maxItems);
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
    ~MultiTypeQueue() {
        // m_queuesMap;
    };
    /**
     * @brief: push message to a queue of t
     *
     * @param event
     */
    void push(Message event)
    {
        if (m_queuesMap.contains(event.type))
        {
            while (m_queuesMap[event.type]->getSize() == m_maxItems)
            {
                std::cout << "waiting" << std::endl;
            }
            m_queuesMap[event.type]->insertMessage(event);
        }
        else
        {
            // TODO: error / logging handling !!!
            std::cout << "error didn't find the queue" << std::endl;
        }
    }

    // FIFO order
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
            result = m_queuesMap[type]->getMessage(type);
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
     */
    void popLastMessage(MessageType type)
    {
        if (m_queuesMap.contains(type))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            // Handle return value
            m_queuesMap[type]->removeMessage();
        }
        else
        {
            // TODO: error / logging handling !!!
            std::cout << "error didn't find the queue" << std::endl;
        }
    }

    // TODO
    /**
     * @brief
     *
     * @param event
     */
    // void updateLast(Message event)
    // {
    // std::unique_lock<std::mutex> lock(m_mtx);
    //     while (m_queuesMap[event.type].empty())
    //     {
    //         m_cv.wait(lock);
    //     }
    //     m_queuesMap[event.type] = event;
    //     m_cv.notify_one();
    // }

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
    bool getItemsByType(MessageType type)
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
