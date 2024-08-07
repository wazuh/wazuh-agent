#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/beast/http/status.hpp>

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;

namespace communicator
{
    class Communicator
    {
    public:
        Communicator(const std::string& uuid,
                     const std::function<std::string(std::string, std::string)> GetStringConfigValue);
        ~Communicator();

        boost::asio::awaitable<void> GetCommandsFromManager();
        boost::asio::awaitable<void> WaitForTokenExpirationAndAuthenticate();
        boost::asio::awaitable<void> StatefulMessageProcessingTask(std::queue<std::string>& messageQueue);
        boost::asio::awaitable<void> StatelessMessageProcessingTask(std::queue<std::string>& messageQueue);

    private:
        std::mutex m_exitMtx;
        std::atomic<bool> m_exitFlag;

        std::string m_uuid;
        std::string m_managerIp;
        std::string m_port;
        long long m_tokenExpTimeInSeconds;
        std::string m_token;

        long GetTokenRemainingSecs() const;

        http::status SendAuthenticationRequest();

        void Stop();
    };
} // namespace communicator
