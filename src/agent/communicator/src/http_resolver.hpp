#include <ihttp_resolver.hpp>

#include <boost/asio.hpp>

#include <string>

namespace http_client
{
    class HttpResolver : public IHttpResolver
    {
    public:
        HttpResolver(const boost::asio::any_io_executor& io_context)
            : m_resolver(io_context)
        {
        }

        boost::asio::ip::tcp::resolver::results_type Resolve(const std::string& host, const std::string& port) override
        {
            return m_resolver.resolve(host, port);
        }

        boost::asio::awaitable<boost::asio::ip::tcp::resolver::results_type>
        AsyncResolve(const std::string& host, const std::string& port) override
        {
            co_return co_await m_resolver.async_resolve(host, port, boost::asio::use_awaitable);
        }

    private:
        boost::asio::ip::tcp::resolver m_resolver;
    };
} // namespace http_client
