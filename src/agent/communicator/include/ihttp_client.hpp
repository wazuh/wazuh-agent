#pragma once

#include <http_request_params.hpp>
#include <ihttp_socket.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace http_client
{
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
