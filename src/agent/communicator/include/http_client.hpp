#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <functional>
#include <string>

namespace http_client
{
    struct HttpRequestParams
    {
        boost::beast::http::verb method;
        std::string url;
        std::string host;
        std::string token;
        std::string body;
        std::string user_pass;
        std::string port;

        HttpRequestParams(boost::beast::http::verb method,
                          const std::string& url,
                          const std::string& host,
                          const std::string& token = "",
                          const std::string& body = "",
                          const std::string& user_pass = "",
                          const std::string& port = "")
            : method(method)
            , url(url)
            , host(host)
            , token(token)
            , body(body)
            , user_pass(user_pass)
            , port(port)
        {
        }
    };

    boost::beast::http::request<boost::beast::http::string_body> CreateHttpRequest(const HttpRequestParams& params);

    boost::beast::http::response<boost::beast::http::dynamic_body> SendRequest(const std::string& managerIp,
                                                                               const std::string& port,
                                                                               const boost::beast::http::verb& method,
                                                                               const std::string& url,
                                                                               const std::string& token,
                                                                               const std::string& body,
                                                                               const std::string& user_pass);

    boost::asio::awaitable<void>
    Co_PerformHttpRequest(boost::asio::ip::tcp::socket& socket,
                          boost::beast::http::request<boost::beast::http::string_body>& req,
                          boost::beast::error_code& ec);

    boost::asio::awaitable<void> Co_MessageProcessingTask(const boost::beast::http::verb method,
                                                          const std::string host,
                                                          const std::string port,
                                                          const std::string target,
                                                          const std::string& token,
                                                          std::function<std::string()> messageGetter);

    boost::beast::http::response<boost::beast::http::dynamic_body> SendHttpRequest(const HttpRequestParams& params);
} // namespace http_client
