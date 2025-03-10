#pragma once

#include <ihttp_resolver.hpp>
#include <logger.hpp>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <exception>
#include <string>

namespace http_client
{
    /// @brief Implementation of IHttpResolver
    class HttpResolver : public IHttpResolver
    {
    public:
        /// @brief Constructs an HttpResolver using the provided executor
        /// @param io_context The io context to use for the resolver
        HttpResolver(const boost::asio::any_io_executor& io_context)
            : m_resolver(io_context)
        {
        }

        /// @copydoc IHttpResolver::Resolve
        boost::asio::ip::tcp::resolver::results_type Resolve(const std::string& host, const std::string& port) override
        {
            try
            {
                boost::system::error_code ec;

                const auto results = m_resolver.resolve(host, port, ec);

                if (ec)
                {
                    throw std::runtime_error(ec.message());
                }

                return results;
            }
            catch (const std::exception& e)
            {
                LogDebug("Failed to resolve host: {} port: {} with error: {}", host, port, e.what());
                return {};
            }
        }

        /// @copydoc IHttpResolver::AsyncResolve
        boost::asio::awaitable<boost::asio::ip::tcp::resolver::results_type>
        AsyncResolve(const std::string& host, const std::string& port) override
        {
            try
            {
                co_return co_await m_resolver.async_resolve(host, port, boost::asio::use_awaitable);
            }
            catch (const std::exception& e)
            {
                LogDebug("Failed to asynchronously resolve host: {} port: {} with error: {}", host, port, e.what());
                co_return boost::asio::ip::tcp::resolver::results_type {};
            }
        }

    private:
        /// @brief The resolver to use for resolving hosts and ports
        boost::asio::ip::tcp::resolver m_resolver;
    };
} // namespace http_client
