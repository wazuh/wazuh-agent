#pragma once

#include <command_entry.hpp>
#include <message.hpp>

#include <boost/asio/awaitable.hpp>
#include <nlohmann/json.hpp>

#include <memory>
#include <optional>
#include <string>
#include <tuple>

class IMultiTypeQueue;

/// @brief Gets messages from a queue and returns them as a JSON string
/// @param multiTypeQueue The queue to get messages from
/// @param messageType The type of messages to get from the queue
/// @param messagesSize Minimum size of messages in bytes to get from the queue
/// @param getMetadataInfo Function to get the agent metadata
/// @return A string containing the messages from the queue
boost::asio::awaitable<std::tuple<int, std::string>>
GetMessagesFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue,
                     MessageType messageType,
                     const size_t messagesSize,
                     std::function<std::string()> getMetadataInfo);

/// @brief Removes a fixed number of messages from the specified queue
/// @param multiTypeQueue The queue from which to remove messages
/// @param messageType The type of messages to remove
/// @param numMessages The number of messages to remove from the queue
void PopMessagesFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue, MessageType messageType, int numMessages);

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
