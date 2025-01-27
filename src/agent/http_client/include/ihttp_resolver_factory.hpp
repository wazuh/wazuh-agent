#pragma once

#include <ihttp_resolver.hpp>

#include <memory>

#include <boost/asio/any_io_executor.hpp>

namespace http_client
{
    /// @brief Interface for HTTP resolver factories
    class IHttpResolverFactory
    {
    public:
        /// @brief Destroys the IHttpResolverFactory
        virtual ~IHttpResolverFactory() = default;

        /// @brief Creates a new IHttpResolver
        /// @param executor The executor to use for the resolver
        /// @return The created IHttpResolver
        virtual std::unique_ptr<IHttpResolver> Create(const boost::asio::any_io_executor& executor) = 0;
    };
} // namespace http_client
