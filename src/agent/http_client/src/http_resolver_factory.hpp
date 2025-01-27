#pragma once

#include <ihttp_resolver_factory.hpp>

#include "http_resolver.hpp"

#include <boost/asio/any_io_executor.hpp>

#include <memory>

namespace http_client
{
    /// @brief Implementation of IHttpResolverFactory
    class HttpResolverFactory : public IHttpResolverFactory
    {
    public:
        /// @brief Creates a new IHttpResolver
        /// @param executor The executor to use for the resolver
        /// @return The created IHttpResolver
        std::unique_ptr<IHttpResolver> Create(const boost::asio::any_io_executor& executor) override
        {
            return std::make_unique<HttpResolver>(executor);
        }
    };
} // namespace http_client
