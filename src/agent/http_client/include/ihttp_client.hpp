#pragma once

#include <http_request_params.hpp>

#include <boost/asio/awaitable.hpp>

#include <ctime>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>

namespace http_client
{
    /// @brief Interface for HTTP client implementations
    ///
    /// This interface provides a standard way of performing HTTP requests and
    /// retrieving the response.
    class IHttpClient
    {
    public:
        /// @brief Destroys the IHttpClient
        virtual ~IHttpClient() = default;

        /// @brief Coroutine to perform an HTTP request
        /// @param token A shared pointer to the authentication token
        /// @param params The parameters for the request
        /// @param bodyGetter Function to retrieve the body
        /// @param onUnauthorized Action to take on unauthorized access
        /// @param connectionRetry Time to wait before retrying the connection
        /// @param batchSize The minimum number of bytes of messages to batch
        /// @param onSuccess Action to take on successful request
        /// @param loopRequestCondition Condition to continue the request loop
        /// @return Awaitable task for the HTTP request
        virtual boost::asio::awaitable<void> Co_PerformHttpRequest(
            std::shared_ptr<std::string> token,
            HttpRequestParams params,
            std::function<boost::asio::awaitable<std::tuple<int, std::string>>(const size_t)> bodyGetter,
            std::function<void()> onUnauthorized,
            std::time_t connectionRetry,
            size_t batchSize,
            std::function<void(const int, const std::string&)> onSuccess = {},
            std::function<bool()> loopRequestCondition = {}) = 0;

        /// @brief Perform an HTTP request and receive the response
        /// @param params The parameters for the request
        /// @return A tuple containing the response status code and body
        virtual std::tuple<int, std::string> PerformHttpRequest(const HttpRequestParams& params) = 0;
    };
} // namespace http_client
