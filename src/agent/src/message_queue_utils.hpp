#pragma once

#include <message.hpp>

#include <boost/asio/awaitable.hpp>

#include <string>

class IMultiTypeQueue;

boost::asio::awaitable<std::string> getMessagesFromQueue(IMultiTypeQueue& multiTypeQueue, MessageType messageType);

void popMessagesFromQueue(IMultiTypeQueue& multiTypeQueue, MessageType messageType);

void pushCommandsToQueue(IMultiTypeQueue& multiTypeQueue, const std::string& commands);
