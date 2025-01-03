#pragma once

#include <ihttp_client.hpp>
#include <ihttp_resolver_factory.hpp>
#include <ihttp_socket_factory.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

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
        /// @param messageGetter Function to get the message body asynchronously
        /// @param onUnauthorized Callback for unauthorized access
        /// @param connectionRetry Time in milliseconds to wait before retrying the connection
        /// @param batchSize The minimum number of bytes of messages to batch
        /// @param onSuccess Callback for successful request completion
        /// @param loopRequestCondition Condition to continue looping requests
        /// @return Awaitable task for the HTTP request
        boost::asio::awaitable<void> Co_PerformHttpRequest(
            std::shared_ptr<std::string> token,
            HttpRequestParams params,
            std::function<boost::asio::awaitable<std::tuple<int, std::string>>(const size_t)> messageGetter,
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

        /// @brief Authenticates using UUID and key
        /// @param serverUrl Server URL for authentication
        /// @param userAgent User agent header
        /// @param uuid Unique user identifier
        /// @param key Authentication key
        /// @param verificationMode Verification mode
        /// @return Authentication token if successful, otherwise nullopt
        std::optional<std::string> AuthenticateWithUuidAndKey(const std::string& serverUrl,
                                                              const std::string& userAgent,
                                                              const std::string& uuid,
                                                              const std::string& key,
                                                              const std::string& verificationMode) override;

        /// @brief Authenticates using username and password
        /// @param serverUrl Server URL for authentication
        /// @param userAgent User agent header
        /// @param user Username for authentication
        /// @param password User password
        /// @param verificationMode Verification mode
        /// @return Authentication token if successful, otherwise nullopt
        std::optional<std::string> AuthenticateWithUserPassword(const std::string& serverUrl,
                                                                const std::string& userAgent,
                                                                const std::string& user,
                                                                const std::string& password,
                                                                const std::string& verificationMode) override;

    private:
        /// @brief Performs an HTTP request with a response handler
        /// @param params Parameters for the request
        /// @param responseHandler Handler for the response
        /// @return The HTTP response
        boost::beast::http::response<boost::beast::http::dynamic_body> PerformHttpRequestInternal(
            const HttpRequestParams& params,
            const std::function<void(std::unique_ptr<IHttpSocket>&,
                                     boost::beast::http::response<boost::beast::http::dynamic_body>&,
                                     boost::system::error_code&,
                                     boost::asio::io_context&)>& responseHandler);

        /// @brief HTTP resolver factory
        std::shared_ptr<IHttpResolverFactory> m_resolverFactory;

        /// @brief HTTP socket factory
        std::shared_ptr<IHttpSocketFactory> m_socketFactory;
    };
} // namespace http_client
