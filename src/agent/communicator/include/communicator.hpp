#pragma once

#include "defs.hpp"
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
        ~Communicator() {}
        int SendAuthenticationRequest();
        int SendRegistrationRequest();

        const std::string& GetToken() const { return m_token; }

    private:
        std::string m_managerIp;
        std::string m_port;
        long long m_tokenExpTime; // expressed in seconds since epoch
        std::string m_token;
        http::response<http::dynamic_body> sendHttpRequest(http::verb method,
                                                           const std::string& url,
                                                           const std::string& token = "",
                                                           const std::string& body = "");
    };
} // namespace communicator
