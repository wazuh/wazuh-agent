#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <string_view>

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

        const std::string& GetToken() const;

    private:
        std::mutex m_exitMtx;
        std::atomic<bool> m_exitFlag;

        std::string m_uuid;
        std::string m_managerIp;
        std::string m_port;
        long long m_tokenExpTimeInSeconds;
        std::string m_token;

        const long GetTokenRemainingSecs() const;

        http::status SendAuthenticationRequest();
        http::response<http::dynamic_body> sendHttpRequest(const http::verb method,
                                                           const std::string& url,
                                                           const std::string& token = "",
                                                           const std::string& body = "");

        void Stop();
    };
} // namespace communicator
