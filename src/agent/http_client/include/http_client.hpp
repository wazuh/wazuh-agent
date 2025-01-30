#pragma once

#include <ihttp_client.hpp>

#include <boost/asio/awaitable.hpp>

#include <functional>
#include <memory>
#include <string>
#include <tuple>

namespace http_client
{
    class IHttpResolverFactory;
    class IHttpSocketFactory;

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

        /// @brief Performs an asynchronous HTTP request
        /// @param params Parameters for the request
        /// @return An awaitable tuple containing the response status code and body
        boost::asio::awaitable<std::tuple<int, std::string>>
        Co_PerformHttpRequest(const HttpRequestParams params) override;

        /// @brief Performs a synchronous HTTP request
        /// @param params Parameters for the request
        /// @return A tuple containing the response status code and body
        std::tuple<int, std::string> PerformHttpRequest(const HttpRequestParams& params) override;

    private:
        /// @brief HTTP resolver factory
        std::shared_ptr<IHttpResolverFactory> m_resolverFactory;

        /// @brief HTTP socket factory
        std::shared_ptr<IHttpSocketFactory> m_socketFactory;
    };
} // namespace http_client
