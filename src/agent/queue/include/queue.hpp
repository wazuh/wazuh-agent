#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <vector>

#include "persistence.hpp"
#include "shared.hpp"

//TODO: move to a configuration setting
constexpr int DEFAULT_MAX = 10;

/**
 * @brief 
 * 
 */
class PersistedQueue
{
public:
    PersistedQueue(MessageType m_queueType, int max_size = DEFAULT_MAX)
        : m_queueType(m_queueType)
        , m_max_size(max_size)
    {
        m_persistenceDest.createOrCheckFile();
    }

    const MessageType& getType() const
    {
        return m_queueType;
    }

    int getSize() const
    {
        return m_size;
    }

    void setSize(int m_size)
    {
        this->m_size = m_size;
    }

    bool insertMessage(Message event)
    {
        m_persistenceDest.writeLine("test-line");
        m_size++;
        return true;
    };

    bool removeMessage()
    {
        m_persistenceDest.removeLine("contenido?");
        m_size--;
        return true;
    };

    Message getMessage(MessageType type)
    {
        return Message(type, m_persistenceDest.readLine(0));
    };

private:
    MessageType m_queueType; // Unnecesary ?
    int m_size = 0;
    int m_max_size;
    Persistence m_persistenceDest {DEFAULT_PERS_PATH};
};

/**
 * 
*/
class MultiTypeQueue
{
private:
    std::unordered_map<MessageType, PersistedQueue> m_queuesMap;
    std::mutex m_mtx;
    std::condition_variable m_cv;
    int m_maxItems;

public:
    MultiTypeQueue(int size = DEFAULT_MAX)
        : m_maxItems(size)
    {
        // Create a vector with 3 PersistedQueue elements
        m_queuesMap = {{MessageType::STATE_LESS, PersistedQueue(MessageType::STATE_LESS, m_maxItems)},
                       {MessageType::STATE_FULL, PersistedQueue(MessageType::STATE_FULL, m_maxItems)},
                       {MessageType::COMMAND, PersistedQueue(MessageType::COMMAND, m_maxItems)}};
    }

    /**
     * @brief: push message to a queue of t
     *
     * @param event
     */
    void push(Message event)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        auto it = m_queuesMap.find(event.type);
        if (it != m_queuesMap.end())
        {
            while (it->second.getSize() == m_maxItems)
            {
                m_cv.wait(lock);
            }
            it->second.insertMessage(event);
            m_cv.notify_one();
        }
        else
        {
            // TODO: error / logging handling !!!
            std::cout << "error didn't find the queue" << std::endl;
        }
    }

    // FIFO order
    Message getLastMessage(MessageType type)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        auto it = m_queuesMap.find(type);
        if (it != m_queuesMap.end())
        {
            auto event = it->second.getMessage(type);
            m_cv.notify_one();
            return event;
        }
        else
        {
            // TODO: error / logging handling !!!
            std::cout << "error didn't find the queue" << std::endl;
        }
    }

    void popLastMessage(MessageType type)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        auto it = m_queuesMap.find(type);
        if (it != m_queuesMap.end())
        {
            // Handle return value
            m_cv.notify_one();
            it->second.removeMessage();
        }
        else
        {
            // TODO: error / logging handling !!!
            std::cout << "error didn't find the queue" << std::endl;
        }
    }

    // void updateLast(Message event) {
    //     std::unique_lock<std::mutex> lock(m_mtx);
    //     while (m_queuesMap[event.type].empty() ) {
    //       m_cv.wait(lock);
    //     }
    //     m_queuesMap[event.type].back() = event;
    //     m_cv.notify_one();
    // }

    bool isEmptyByType(MessageType type)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        auto it = m_queuesMap.find(type);
        if (it != m_queuesMap.end())
        {
            return it->second.getSize() == 0;
        }
        else
        {
            // TODO: error / logging handling !!!
            std::cout << "error didn't find the queue" << std::endl;
        }
        return false;
    }
};
