#pragma once

#include <ihttp_socket_factory.hpp>

#include "http_socket.hpp"
#include "https_socket.hpp"

#include <boost/asio/any_io_executor.hpp>

#include <memory>

namespace http_client
{
    class HttpSocketFactory : public IHttpSocketFactory
    {
    public:
        std::unique_ptr<IHttpSocket> Create(const boost::asio::any_io_executor& executor) override
        {
            return std::make_unique<HttpSocket>(executor);
        }
    };
} // namespace http_client
