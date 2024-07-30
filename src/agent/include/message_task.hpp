#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <queue>
#include <string>

boost::asio::awaitable<void> StatefulMessageProcessingTask(const std::string& manager_ip, const std::string& port, const std::string& token, std::queue<std::string>& messageQueue);

boost::asio::awaitable<void> StatelessMessageProcessingTask(const std::string& manager_ip, const std::string& port, const std::string& token, std::queue<std::string>& messageQueue);
