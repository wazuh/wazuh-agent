#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <queue>
#include <string>

boost::asio::awaitable<void> StatefulMessageProcessingTask(std::queue<std::string>& messageQueue);

boost::asio::awaitable<void> StatelessMessageProcessingTask(std::queue<std::string>& messageQueue);
