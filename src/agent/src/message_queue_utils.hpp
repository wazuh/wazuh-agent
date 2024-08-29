#pragma once

#include <message.hpp>

#include <boost/asio/awaitable.hpp>

#include <memory>
#include <string>

class IMultiTypeQueue;

boost::asio::awaitable<std::string> GetMessagesFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue,
                                                         MessageType messageType);

void PopMessagesFromQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue, MessageType messageType);

void PushCommandsToQueue(std::shared_ptr<IMultiTypeQueue> multiTypeQueue, const std::string& commands);
