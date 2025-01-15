#pragma once

#include <message.hpp>

#include <boost/asio/awaitable.hpp>

#include <string>
#include <vector>

/// @brief Interface for a multi-type message queue.
///
/// This interface defines the operations for managing messages in a queue
/// that supports multiple message types. Classes that implement this interface
/// must provide the functionality described by these methods.
class IMultiTypeQueue
{
public:
    /// @brief Virtual destructor.
    virtual ~IMultiTypeQueue() = default;

    /// @brief Pushes a single message onto the queue.
    /// @param message The message to be pushed.
    /// @param shouldWait If true, the function waits until the message is pushed.
    /// @return int The number of messages pushed.
    virtual int push(Message message, bool shouldWait = false) = 0;

    /// @brief Pushes a single message onto the queue asynchronously.
    /// @param message The message to be pushed.
    /// @return boost::asio::awaitable<int> The number of messages pushed.
    virtual boost::asio::awaitable<int> pushAwaitable(Message message) = 0;

    /// @brief Pushes a vector of messages onto the queue.
    /// @param messages The vector of messages to be pushed.
    /// @return int The number of messages pushed.
    virtual int push(std::vector<Message> messages) = 0;

    /// @brief Retrieves the next message from the queue.
    /// @param type The type of the queue to use as the source.
    /// @param moduleName The name of the module requesting the message.
    /// @param moduleType The type of the module requesting the messages.
    /// @return Message The next message from the queue.
    virtual Message getNext(MessageType type, const std::string moduleName = "", const std::string moduleType = "") = 0;

    /// @brief Retrieves the next Bytes of messages from the queue asynchronously.
    /// @param type The type of the queue to use as the source.
    /// @param messageQuantity In bytes of messages.
    /// @param moduleName The name of the module requesting the message.
    /// @param moduleType The type of the module requesting the messages.
    /// @return boost::asio::awaitable<std::vector<Message>> Awaitable object representing the next N messages.
    virtual boost::asio::awaitable<std::vector<Message>> getNextBytesAwaitable(MessageType type,
                                                                               const size_t messageQuantity,
                                                                               const std::string moduleName = "",
                                                                               const std::string moduleType = "") = 0;

    /// @brief Retrieves the next N messages from the queue.
    /// @param type The type of the queue to use as the source.
    /// @param messageQuantity The quantity of bytes of messages to return.
    /// @param moduleName The name of the module requesting the messages.
    /// @param moduleType The type of the module requesting the messages.
    /// @return std::vector<Message> A vector of messages fetched from the queue.
    virtual std::vector<Message> getNextBytes(MessageType type,
                                              const size_t messageQuantity,
                                              const std::string moduleName = "",
                                              const std::string moduleType = "") = 0;

    /// @brief Deletes a message from the queue.
    /// @param type The type of the queue from which to pop the message.
    /// @param moduleName The name of the module requesting the pop.
    /// @param moduleType The type of the module requesting the pop.
    /// @return true If the message was popped successfully.
    /// @return false If the message could not be popped.
    virtual bool pop(MessageType type, const std::string moduleName = "", const std::string moduleType = "") = 0;

    /// @brief Deletes N messages from the queue.
    /// @param type The type of the queue from which to pop the messages.
    /// @param messageQuantity The quantity of messages to pop.
    /// @param moduleName The name of the module requesting the pop.
    /// @param moduleType The type of the module requesting the pop.
    /// @return int The number of messages deleted.
    virtual int popN(MessageType type,
                     int messageQuantity,
                     const std::string moduleName = "",
                     const std::string moduleType = "") = 0;

    /// @brief Checks if a queue is empty.
    /// @param type The type of the queue.
    /// @param moduleName The name of the module requesting the check.
    /// @param moduleType The type of the module requesting the check.
    /// @return true If the queue is empty.
    /// @return false If the queue is not empty.
    virtual bool isEmpty(MessageType type, const std::string moduleName = "", const std::string moduleType = "") = 0;

    /// @brief Checks if a queue is full
    /// @param type The type of the queue.
    /// @param moduleName The name of the module requesting the check.
    /// @param moduleType The type of the module requesting the check.
    /// @return true If the queue is full.
    /// @return false If the queue is not full.
    virtual bool isFull(MessageType type, const std::string moduleName = "", const std::string moduleType = "") = 0;

    /// @brief Returns the number of items stored in the queue.
    /// @param type The type of the queue.
    /// @param moduleName The name of the module requesting the count.
    /// @param moduleType The type of the module requesting the count.
    /// @return int The number of items in the queue.
    virtual int storedItems(MessageType type, const std::string moduleName = "", const std::string moduleType = "") = 0;

    /// @brief Returns the size of the queue per type
    /// @param type The type of the queue.
    /// @return size_t The size of the queue.
    virtual size_t sizePerType(MessageType type) = 0;
};
