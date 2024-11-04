#pragma once

#include <ihttp_socket.hpp>

#include <boost/asio/any_io_executor.hpp>

#include <memory>

namespace http_client
{
    /// @brief Interface for HTTP socket factories
    class IHttpSocketFactory
    {
    public:
        /// @brief Destroys the IHttpSocketFactory
        virtual ~IHttpSocketFactory() = default;

        /// @brief Creates a new IHttpSocket
        /// @param executor The executor to use for the socket
        /// @param use_https Indicates whether to use HTTPS
        /// @return The created IHttpSocket
        virtual std::unique_ptr<IHttpSocket> Create(const boost::asio::any_io_executor& executor,
                                                    const bool use_https) = 0;
    };
} // namespace http_client
