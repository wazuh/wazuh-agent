#pragma once

#include <ihttp_client.hpp>
#include <ihttp_resolver_factory.hpp>
#include <ihttp_socket_factory.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/beast/http.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace http_client
{
    /// @brief HTTP client implementation
    ///
    /// This class implements the IHttpClient interface, providing
    /// functionality for creating and performing HTTP requests.
    class HttpClient : public IHttpClient
    {
    public:
        /// @brief Constructs an HttpClient with optional factories
        /// @param resolverFactory Factory to create HTTP resolvers
        /// @param socketFactory Factory to create HTTP sockets
        HttpClient(std::shared_ptr<IHttpResolverFactory> resolverFactory = nullptr,
                   std::shared_ptr<IHttpSocketFactory> socketFactory = nullptr);

        /// @brief Creates an HTTP request
        /// @param params Parameters for constructing the request
        /// @return The constructed HTTP request
        boost::beast::http::request<boost::beast::http::string_body>
        CreateHttpRequest(const HttpRequestParams& params) override;

        /// @brief Performs an asynchronous HTTP request
        /// @param token Authorization token
        /// @param params Request parameters
        /// @param bodyGetter Function to get the body asynchronously
        /// @param onUnauthorized Callback for unauthorized access
        /// @param connectionRetry Time in milliseconds to wait before retrying the connection
        /// @param batchSize The minimum number of bytes of messages to batch
        /// @param onSuccess Callback for successful request completion
        /// @param loopRequestCondition Condition to continue looping requests
        /// @return Awaitable task for the HTTP request
        boost::asio::awaitable<void> Co_PerformHttpRequest(
            std::shared_ptr<std::string> token,
            HttpRequestParams params,
            std::function<boost::asio::awaitable<std::tuple<int, std::string>>(const size_t)> bodyGetter,
            std::function<void()> onUnauthorized,
            std::time_t connectionRetry,
            size_t batchSize,
            std::function<void(const int, const std::string&)> onSuccess = {},
            std::function<bool()> loopRequestCondition = {}) override;

        /// @brief Performs a synchronous HTTP request
        /// @param params Parameters for the request
        /// @return The HTTP response
        boost::beast::http::response<boost::beast::http::dynamic_body>
        PerformHttpRequest(const HttpRequestParams& params) override;

    private:
        /// @brief HTTP resolver factory
        std::shared_ptr<IHttpResolverFactory> m_resolverFactory;

        /// @brief HTTP socket factory
        std::shared_ptr<IHttpSocketFactory> m_socketFactory;
    };
} // namespace http_client
