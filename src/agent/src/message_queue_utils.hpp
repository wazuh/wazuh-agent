#pragma once

#include <command.hpp>
#include <message.hpp>

#include <boost/asio/awaitable.hpp>

#include <memory>
#include <optional>
#include <string>

class IMultiTypeQueue;

boost::asio::awaitable<std::string> GetMessagesFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue,
                                                         MessageType messageType);

void PopMessagesFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue, MessageType messageType);

void PushCommandsToQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue, const std::string& commands);

std::optional<command_store::Command> GetCommandFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue);

void PopCommandFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue);
