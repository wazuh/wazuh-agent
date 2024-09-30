#pragma once

#include <message.hpp>
#include <module_command/command_entry.hpp>

#include <boost/asio/awaitable.hpp>

#include <memory>
#include <optional>
#include <string>

class IMultiTypeQueue;

boost::asio::awaitable<std::string> GetMessagesFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue,
                                                         MessageType messageType);

void PopMessagesFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue, MessageType messageType);

void PushCommandsToQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue, const std::string& commands);

std::optional<module_command::CommandEntry> GetCommandFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue);

void PopCommandFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue);
