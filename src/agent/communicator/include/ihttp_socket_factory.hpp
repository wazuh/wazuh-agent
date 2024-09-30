#pragma once

#include <ihttp_socket.hpp>

#include <boost/asio/any_io_executor.hpp>

#include <memory>

namespace http_client
{
    class IHttpSocketFactory
    {
    public:
        virtual ~IHttpSocketFactory() = default;
        virtual std::unique_ptr<IHttpSocket> Create(const boost::asio::any_io_executor& executor,
                                                    const bool use_https) = 0;
    };
} // namespace http_client
