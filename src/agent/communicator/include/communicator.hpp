#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <functional>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;

namespace communicator
{
    class Communicator
    {
    public:
        Communicator(const std::function<std::string(std::string, std::string)> GetStringConfigValue);
        ~Communicator();
        int SendAuthenticationRequest();
        int SendRegistrationRequest();

        boost::asio::awaitable<void> WaitForTokenExpirationAndAuthenticate();

        const std::string& GetToken() const;
        const long GetTokenRemainingSecs() const;
        void Stop();

    private:
        std::mutex m_exitMtx;
        std::atomic<bool> m_exitFlag;

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
