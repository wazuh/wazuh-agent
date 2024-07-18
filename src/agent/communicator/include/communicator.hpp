#pragma once

#include "defs.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;

namespace communicator
{
    class Communicator
    {
    public:
        Communicator();
        ~Communicator() {}
        int SendAuthenticationRequest();
        int SendRegistrationRequest();

    private:
        std::string m_token;
        http::response<http::dynamic_body> sendHttpRequest(http::verb method,
                                                           const std::string& url,
                                                           const std::string& token = "",
                                                           const std::string& body = "");
    };
} // namespace communicator
