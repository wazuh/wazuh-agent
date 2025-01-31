#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <string>

namespace http_client
{
    /// @brief Interface for HTTP resolvers
    class IHttpResolver
    {
    public:
        /// @brief Destroys the IHttpResolver
        virtual ~IHttpResolver() = default;

        /// @brief Resolves a host and port to a list of endpoints
        /// @param host The host to resolve
        /// @param port The port to resolve
        /// @return Resolved endpoints
        virtual boost::asio::ip::tcp::resolver::results_type Resolve(const std::string& host,
                                                                     const std::string& port) = 0;

        /// @brief Asynchronously resolves a host and port to a list of endpoints
        /// @param host The host to resolve
        /// @param port The port to resolve
        /// @return Awaitable resolved endpoints
        virtual boost::asio::awaitable<boost::asio::ip::tcp::resolver::results_type>
        AsyncResolve(const std::string& host, const std::string& port) = 0;
    };
} // namespace http_client
