#pragma once

#include <http_request_params.hpp>

#include <boost/asio/awaitable.hpp>

#include <functional>
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
        /// @param params The parameters for the request
        /// @return An awaitable tuple containing the response status code and body
        virtual boost::asio::awaitable<std::tuple<int, std::string>>
        Co_PerformHttpRequest(const HttpRequestParams params) = 0;

        /// @brief Perform an HTTP request and receive the response
        /// @param params The parameters for the request
        /// @return A tuple containing the response status code and body
        virtual std::tuple<int, std::string> PerformHttpRequest(const HttpRequestParams& params) = 0;
    };
} // namespace http_client
