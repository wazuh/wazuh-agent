#pragma once

#include <ihttp_client.hpp>
#include <ihttp_resolver_factory.hpp>
#include <ihttp_socket.hpp>
#include <ihttp_socket_factory.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace http_client
{
    class HttpClient : public IHttpClient
    {
    public:
        HttpClient(std::shared_ptr<IHttpResolverFactory> resolverFactory = nullptr,
                   std::shared_ptr<IHttpSocketFactory> socketFactory = nullptr);

        boost::beast::http::request<boost::beast::http::string_body>
        CreateHttpRequest(const HttpRequestParams& params) override;

        boost::asio::awaitable<void>
        Co_PerformHttpRequest(const std::string& token,
                              HttpRequestParams params,
                              std::function<boost::asio::awaitable<std::string>()> messageGetter,
                              std::function<void()> onUnauthorized,
                              std::function<void(const std::string&)> onSuccess = {},
                              std::function<bool()> loopRequestCondition = {}) override;

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

    private:
        std::shared_ptr<IHttpResolverFactory> m_resolverFactory;
        std::shared_ptr<IHttpSocketFactory> m_socketFactory;
    };
} // namespace http_client
