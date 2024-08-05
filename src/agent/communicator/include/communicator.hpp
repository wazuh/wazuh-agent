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
    const std::string_view uuidKey = "uuid";
    const std::string_view kUUID = "agent_uuid";

    class Communicator
    {
    public:
        Communicator(const std::string& uuid,
                     const std::function<std::string(std::string, std::string)> GetStringConfigValue);
        ~Communicator();

        http::status SendAuthenticationRequest();

        boost::asio::awaitable<void> GetCommandsFromManager();
        boost::asio::awaitable<void> WaitForTokenExpirationAndAuthenticate();

        boost::asio::awaitable<void> StatefulMessageProcessingTask(const std::string& manager_ip,
                                                                   const std::string& port,
                                                                   const std::string& token,
                                                                   std::queue<std::string>& messageQueue);

        boost::asio::awaitable<void> StatelessMessageProcessingTask(const std::string& manager_ip,
                                                                    const std::string& port,
                                                                    const std::string& token,
                                                                    std::queue<std::string>& messageQueue);

        const std::string& GetToken() const;
        const long GetTokenRemainingSecs() const;
        void Stop();

    private:
        std::mutex m_exitMtx;
        std::atomic<bool> m_exitFlag;

        std::string m_uuid;
        std::string m_managerIp;
        std::string m_port;
        long long m_tokenExpTimeInSeconds;
        std::string m_token;
        http::response<http::dynamic_body> sendHttpRequest(http::verb method,
                                                           const std::string& url,
                                                           const std::string& token = "",
                                                           const std::string& body = "");
    };
} // namespace communicator
