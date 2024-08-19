#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/http/status.hpp>

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

namespace communicator
{
    class Communicator
    {
    public:
        Communicator(const std::string& uuid,
                     const std::string& key,
                     const std::function<std::string(std::string, std::string)> GetStringConfigValue);

        boost::asio::awaitable<void> WaitForTokenExpirationAndAuthenticate();
        boost::asio::awaitable<void> GetCommandsFromManager(std::function<void(const std::string&)> onSuccess);
        boost::asio::awaitable<void>
        StatefulMessageProcessingTask(std::function<boost::asio::awaitable<std::string>()> getMessages,
                                      std::function<void(const std::string&)> onSuccess);
        boost::asio::awaitable<void>
        StatelessMessageProcessingTask(std::function<boost::asio::awaitable<std::string>()> getMessages,
                                       std::function<void(const std::string&)> onSuccess);

    private:
        long GetTokenRemainingSecs() const;

        boost::beast::http::status SendAuthenticationRequest();

        void TryReAuthenticate();

        std::mutex m_reAuthMutex;
        std::atomic<bool> m_isReAuthenticating = false;

        std::string m_managerIp;
        std::string m_port;
        std::string m_uuid;
        std::string m_key;
        std::string m_token;
        long long m_tokenExpTimeInSeconds = 0;
        std::unique_ptr<boost::asio::steady_timer> m_tokenExpTimer;
    };
} // namespace communicator
