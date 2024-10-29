#pragma once

#include <ihttp_socket.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/url.hpp>

#include <functional>
#include <memory>
#include <optional>
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
                          const std::string& host,
                          const std::string& endpoint,
                          const std::string& token = "",
                          const std::string& user_pass = "",
                          const std::string& body = "")
            : Method(method)
            , Host(host)
            , Endpoint(endpoint)
            , Token(token)
            , User_pass(user_pass)
            , Body(body)
        {
            boost::urls::url_view url(host);

            Use_Https = url.scheme().empty() || url.scheme() == "https";
            Host = url.host();
            Port = url.port();
        }

        bool operator==(const HttpRequestParams& other) const
        {
            return Method == other.Method && Host == other.Host && Port == other.Port && Endpoint == other.Endpoint &&
                   Token == other.Token && User_pass == other.User_pass && Body == other.Body &&
                   Use_Https == other.Use_Https;
        }
    };

    class IHttpClient
    {
    public:
        virtual ~IHttpClient() = default;

        virtual boost::beast::http::request<boost::beast::http::string_body>
        CreateHttpRequest(const HttpRequestParams& params) = 0;

        virtual boost::asio::awaitable<void>
        Co_PerformHttpRequest(std::shared_ptr<std::string> token,
                              HttpRequestParams params,
                              std::function<boost::asio::awaitable<std::string>()> messageGetter,
                              std::function<void()> onUnauthorized,
                              std::function<void(const std::string&)> onSuccess = {},
                              std::function<bool()> loopRequestCondition = {}) = 0;

        virtual boost::beast::http::response<boost::beast::http::dynamic_body>
        PerformHttpRequest(const HttpRequestParams& params) = 0;

        virtual std::optional<std::string>
        AuthenticateWithUuidAndKey(const std::string& serverUrl, const std::string& uuid, const std::string& key) = 0;

        virtual std::optional<std::string> AuthenticateWithUserPassword(const std::string& serverUrl,
                                                                        const std::string& user,
                                                                        const std::string& password) = 0;

        virtual boost::beast::http::response<boost::beast::http::dynamic_body>
        PerformHttpRequestDownload(const HttpRequestParams& params, const std::string& dstFilePath) = 0;
    };
} // namespace http_client
