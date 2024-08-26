#pragma once

#include <ihttp_resolver.hpp>

#include <memory>

#include <boost/asio/any_io_executor.hpp>

namespace http_client
{
    class IHttpResolverFactory
    {
    public:
        virtual ~IHttpResolverFactory() = default;
        virtual std::unique_ptr<IHttpResolver> Create(const boost::asio::any_io_executor& executor) = 0;
    };
} // namespace http_client
