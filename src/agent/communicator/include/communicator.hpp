#pragma once

#include <ihttp_client.hpp>

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
        Communicator(std::unique_ptr<http_client::IHttpClient> httpClient,
                     std::string uuid,
                     std::string key,
                     const std::function<std::string(std::string, std::string)>& getStringConfigValue);

        boost::asio::awaitable<void> WaitForTokenExpirationAndAuthenticate();
        boost::asio::awaitable<void> GetCommandsFromManager(std::function<void(const std::string&)> onSuccess);
        boost::asio::awaitable<void>
        StatefulMessageProcessingTask(std::function<boost::asio::awaitable<std::string>()> getMessages,
                                      std::function<void(const std::string&)> onSuccess);
        boost::asio::awaitable<void>
        StatelessMessageProcessingTask(std::function<boost::asio::awaitable<std::string>()> getMessages,
                                       std::function<void(const std::string&)> onSuccess);
        bool GetGroupConfigurationFromManager(const std::string& groupName, const std::string& dstFilePath);

        void Stop();

    private:
        long GetTokenRemainingSecs() const;

        boost::beast::http::status SendAuthenticationRequest();

        void TryReAuthenticate();

        std::atomic<bool> m_keepRunning = true;
        std::unique_ptr<http_client::IHttpClient> m_httpClient;

        std::mutex m_reAuthMutex;
        std::atomic<bool> m_isReAuthenticating = false;

        std::string m_serverUrl;
        std::string m_registrationUrl;
        std::string m_uuid;
        std::string m_key;
        std::shared_ptr<std::string> m_token;
        long long m_tokenExpTimeInSeconds = 0;
        std::unique_ptr<boost::asio::steady_timer> m_tokenExpTimer;
    };
} // namespace communicator
