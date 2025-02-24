#pragma once

#include <configuration_parser.hpp>
#include <imultitype_queue.hpp>
#include <istorage.hpp>

#include <boost/asio/awaitable.hpp>

#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class Storage;

namespace
{
    // table names
    const std::string STATELESS_TABLE_NAME = "STATELESS";
    const std::string STATEFUL_TABLE_NAME = "STATEFUL";
    const std::string COMMAND_TABLE_NAME = "COMMAND";
} // namespace

/// @brief MultiTypeQueue implementation that handles multiple types of messages.
///
/// This class implements the IMultiTypeQueue interface to provide a queue
/// that can handle different message types such as STATELESS, STATEFUL, and COMMAND.
class MultiTypeQueue : public IMultiTypeQueue
{
private:
    const std::vector<std::string> m_vMessageTypeStrings {
        STATELESS_TABLE_NAME, STATEFUL_TABLE_NAME, COMMAND_TABLE_NAME};
    const std::map<MessageType, std::string> m_mapMessageTypeName {
        {MessageType::STATELESS, STATELESS_TABLE_NAME},
        {MessageType::STATEFUL, STATEFUL_TABLE_NAME},
        {MessageType::COMMAND, COMMAND_TABLE_NAME},
    };

    /// @brief maximun quantity of message to stored on the queue
    size_t m_maxItems;

    /// @brief timeout in milliseconds for refreshing the queue status
    const std::chrono::milliseconds m_timeout;

    /// @brief Unique pointer to the Storage
    std::unique_ptr<IStorage> m_persistenceDest;

    /// @brief mutex for protecting the queue access
    std::mutex m_mtx;

    /// @brief condition variable related to the mutex
    std::condition_variable m_cv;

    /// @brief Time between batch requests
    std::time_t m_batchInterval;

public:
    /// @brief Constructor
    /// @param configurationParser Pointer to the configuration parser
    MultiTypeQueue(std::shared_ptr<configuration::ConfigurationParser> configurationParser,
                   std::unique_ptr<IStorage> persistenceDest = nullptr);

    /// @brief Delete copy constructor
    MultiTypeQueue(const MultiTypeQueue&) = delete;

    /// @brief Delete copy assignment operator
    MultiTypeQueue& operator=(const MultiTypeQueue&) = delete;

    /// @brief Delete move constructor
    MultiTypeQueue(MultiTypeQueue&&) = delete;

    /// @brief Delete move assignment operator
    MultiTypeQueue& operator=(MultiTypeQueue&&) = delete;

    /// @brief Destructor
    ~MultiTypeQueue() override;

    /// @copydoc IMultiTypeQueue::push(Message, bool)
    int push(Message message, bool shouldWait = false) override;

    /// @copydoc IMultiTypeQueue::pushAwaitable(Message)
    boost::asio::awaitable<int> pushAwaitable(Message message) override;

    /// @copydoc IMultiTypeQueue::push(std::vector<Message>)
    int push(std::vector<Message> messages) override;

    /// @copydoc IMultiTypeQueue::getNext(MessageType, const std::string, const std::string)
    Message getNext(MessageType type, const std::string moduleName = "", const std::string moduleType = "") override;

    /// @copydoc IMultiTypeQueue::getNextBytesAwaitable(MessageType type, const size_t
    /// messageQuantity, const std::string moduleName, const std::string moduleType)
    boost::asio::awaitable<std::vector<Message>> getNextBytesAwaitable(MessageType type,
                                                                       const size_t messageQuantity,
                                                                       const std::string moduleName = "",
                                                                       const std::string moduleType = "") override;

    /// @copydoc IMultiTypeQueue::getNextBytes(MessageType, size_t, const std::string, const std::string)
    std::vector<Message> getNextBytes(MessageType type,
                                      const size_t messageQuantity,
                                      const std::string moduleName = "",
                                      const std::string moduleType = "") override;

    /// @copydoc IMultiTypeQueue::pop(MessageType, const std::string, const std::string)
    bool pop(MessageType type, const std::string moduleName = "", const std::string moduleType = "") override;

    /// @copydoc IMultiTypeQueue::popN(MessageType, int, const std::string, const std::string)
    int popN(MessageType type,
             int messageQuantity,
             const std::string moduleName = "",
             const std::string moduleType = "") override;

    /// @copydoc IMultiTypeQueue::isEmpty(MessageType, const std::string, const std::string)
    bool isEmpty(MessageType type, const std::string moduleName = "", const std::string moduleType = "") override;

    /// @copydoc IMultiTypeQueue::isFull(MessageType, const std::string, const std::string)
    bool isFull(MessageType type, const std::string moduleName = "", const std::string moduleType = "") override;

    /// @copydoc IMultiTypeQueue::storedItems(MessageType, const std::string, const std::string)
    int storedItems(MessageType type, const std::string moduleName = "", const std::string moduleType = "") override;

    /// @copydoc IMultiTypeQueue::sizePerType(MessageType type)
    size_t sizePerType(MessageType type) override;
};
