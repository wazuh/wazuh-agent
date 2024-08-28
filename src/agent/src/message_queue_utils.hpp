#pragma once

#include <message.hpp>

#include <boost/asio/awaitable.hpp>

#include <string>

class IMultiTypeQueue;

boost::asio::awaitable<std::string> GetMessagesFromQueue(IMultiTypeQueue& multiTypeQueue, MessageType messageType);

void PopMessagesFromQueue(IMultiTypeQueue& multiTypeQueue, MessageType messageType);

void PushCommandsToQueue(IMultiTypeQueue& multiTypeQueue, const std::string& commands);
