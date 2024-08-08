#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <functional>
#include <optional>
#include <string>

namespace http_client
{
    struct HttpRequestParams
    {
        boost::beast::http::verb method;
        std::string host;
        std::string port;
        std::string endpoint;
        std::string token;
        std::string user_pass;
        std::string body;

        HttpRequestParams(boost::beast::http::verb method,
                          const std::string& host,
                          const std::string& port,
                          const std::string& endpoint,
                          const std::string& token = "",
                          const std::string& user_pass = "",
                          const std::string& body = "")
            : method(method)
            , host(host)
            , port(port)
            , endpoint(endpoint)
            , token(token)
            , user_pass(user_pass)
            , body(body)
        {
        }
    };

    boost::beast::http::request<boost::beast::http::string_body> CreateHttpRequest(const HttpRequestParams& params);

    boost::asio::awaitable<void>
    Co_PerformHttpRequest(boost::asio::ip::tcp::socket& socket,
                          boost::beast::http::request<boost::beast::http::string_body>& req,
                          boost::beast::error_code& ec,
                          std::function<void()> onUnauthorized,
                          std::function<void(const std::string&)> onSuccess = {});

    boost::asio::awaitable<void> Co_MessageProcessingTask(const std::string& token,
                                                          HttpRequestParams params,
                                                          std::function<std::string()> messageGetter,
                                                          std::function<void()> onUnauthorized,
                                                          std::function<void(const std::string&)> onSuccess = {});

    boost::beast::http::response<boost::beast::http::dynamic_body> PerformHttpRequest(const HttpRequestParams& params);

    std::optional<std::string>
    AuthenticateWithUuid(const std::string& host, const std::string& port, const std::string& uuid);
    std::optional<std::string> AuthenticateWithUserPassword(const std::string& host,
                                                            const std::string& port,
                                                            const std::string& user,
                                                            const std::string& password);
} // namespace http_client
