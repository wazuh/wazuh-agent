#pragma once

#include <ihttp_socket_factory.hpp>

#include "http_socket.hpp"
#include "https_socket.hpp"

#include <boost/asio/any_io_executor.hpp>

#include <memory>

namespace http_client
{
    /// @brief Implementation of IHttpSocketFactory
    class HttpSocketFactory : public IHttpSocketFactory
    {
    public:
        /// @brief Creates a new IHttpSocket
        /// @param executor The executor to use for the socket
        /// @param use_https Indicates whether to use HTTPS
        /// @return The created IHttpSocket
        std::unique_ptr<IHttpSocket> Create(const boost::asio::any_io_executor& executor, const bool use_https) override
        {
            if (use_https)
            {
                return std::make_unique<HttpsSocket>(executor);
            }

            return std::make_unique<HttpSocket>(executor);
        }
    };
} // namespace http_client
