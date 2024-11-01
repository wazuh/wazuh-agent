#pragma once

#include <message.hpp>
#include <module_command/command_entry.hpp>

#include <boost/asio/awaitable.hpp>

#include <memory>
#include <optional>
#include <string>

class IMultiTypeQueue;

/// @brief Gets messages from a queue and returns them as a JSON string
/// @param multiTypeQueue The queue to get messages from
/// @param messageType The type of messages to get from the queue
/// @return A JSON string containing the messages from the queue
boost::asio::awaitable<std::string> GetMessagesFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue,
                                                         MessageType messageType);

/// @brief Removes a fixed number of messages from the specified queue
/// @param multiTypeQueue The queue from which to remove messages
/// @param messageType The type of messages to remove
void PopMessagesFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue, MessageType messageType);

/// @brief Pushes a batch of commands to the specified queue
/// @param multiTypeQueue The queue to push commands to
/// @param commands A JSON string containing the commands to push
void PushCommandsToQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue, const std::string& commands);

/// @brief Retrieves the next command from the queue, if available
/// @param multiTypeQueue The queue to retrieve the command from
/// @return An optional containing the next command entry, or nullopt if the queue is empty
std::optional<module_command::CommandEntry> GetCommandFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue);

/// @brief Removes the next command from the specified queue
/// @param multiTypeQueue The queue from which to remove the command
void PopCommandFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue);
