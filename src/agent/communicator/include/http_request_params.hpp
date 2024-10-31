#pragma once

#include <boost/beast.hpp>

#include <string>

namespace http_client
{
    struct HttpRequestParams
    {
        boost::beast::http::verb Method;
        std::string Host;
        std::string Port;
        std::string Endpoint;
        bool Use_Https;
        std::string Token;
        std::string User_pass;
        std::string Body;

        HttpRequestParams(boost::beast::http::verb method,
                          const std::string& serverUrl,
                          std::string endpoint,
                          std::string token = "",
                          std::string userPass = "",
                          std::string body = "");

        bool operator==(const HttpRequestParams& other) const;
    };
} // namespace http_client
