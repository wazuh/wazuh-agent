#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/beast/http/status.hpp>

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <string>

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
        long GetTokenRemainingSecs() const;

        boost::beast::http::status SendAuthenticationRequest();

        void TryReAuthenticate();

        std::mutex m_exitMtx;
        std::atomic<bool> m_exitFlag = false;

        std::mutex m_reAuthMutex;
        std::atomic<bool> m_isReAuthenticating = false;

        std::string m_managerIp;
        std::string m_port;
        std::string m_uuid;
        std::string m_token;
        long long m_tokenExpTimeInSeconds = 0;
    };
} // namespace communicator
