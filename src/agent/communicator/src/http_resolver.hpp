#include <ihttp_resolver.hpp>

#include <boost/asio.hpp>

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

        /// @brief Resolves a host and port to a list of endpoints
        /// @param host The host to resolve
        /// @param port The port to resolve
        /// @return Resolved endpoints
        boost::asio::ip::tcp::resolver::results_type Resolve(const std::string& host, const std::string& port) override
        {
            return m_resolver.resolve(host, port);
        }

        /// @brief Asynchronously resolves a host and port to a list of endpoints
        /// @param host The host to resolve
        /// @param port The port to resolve
        /// @return Awaitable resolved endpoints
        boost::asio::awaitable<boost::asio::ip::tcp::resolver::results_type>
        AsyncResolve(const std::string& host, const std::string& port) override
        {
            co_return co_await m_resolver.async_resolve(host, port, boost::asio::use_awaitable);
        }

    private:
        /// @brief The resolver to use for resolving hosts and ports
        boost::asio::ip::tcp::resolver m_resolver;
    };
} // namespace http_client
