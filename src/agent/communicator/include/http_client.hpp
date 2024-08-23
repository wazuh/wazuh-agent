#pragma once

#include <ihttp_client.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <functional>
#include <optional>
#include <string>

namespace http_client
{
    class HttpClient : public IHttpClient
    {
    public:
        boost::beast::http::request<boost::beast::http::string_body>
        CreateHttpRequest(const HttpRequestParams& params) override;

        boost::asio::awaitable<void>
        Co_PerformHttpRequest(boost::asio::ip::tcp::socket& socket,
                              boost::beast::http::request<boost::beast::http::string_body>& req,
                              boost::beast::error_code& ec,
                              std::function<void()> onUnauthorized,
                              std::function<void(const std::string&)> onSuccess = {}) override;

        boost::asio::awaitable<void>
        Co_MessageProcessingTask(const std::string& token,
                                 HttpRequestParams params,
                                 std::function<boost::asio::awaitable<std::string>()> messageGetter,
                                 std::function<void()> onUnauthorized,
                                 std::function<void(const std::string&)> onSuccess = {}) override;

        boost::beast::http::response<boost::beast::http::dynamic_body>
        PerformHttpRequest(const HttpRequestParams& params) override;

        std::optional<std::string> AuthenticateWithUuidAndKey(const std::string& host,
                                                              const std::string& port,
                                                              const std::string& uuid,
                                                              const std::string& key) override;

        std::optional<std::string> AuthenticateWithUserPassword(const std::string& host,
                                                                const std::string& port,
                                                                const std::string& user,
                                                                const std::string& password) override;
    };
} // namespace http_client
