#include "https_socket.hpp"

#include "http_client_utils.hpp"
#include <ihttp_socket.hpp>
#include <logger.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/system/error_code.hpp>

#include <exception>
#include <string>

namespace http_client
{
    HttpsSocket::HttpsSocket(const boost::asio::any_io_executor& ioContext)
        : m_ctx(boost::asio::ssl::context::sslv23)
        , m_ssl_socket(ioContext, m_ctx)
    {
        m_ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    }

    void HttpsSocket::Connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                              boost::system::error_code& ec)
    {
        try
        {
            boost::asio::connect(m_ssl_socket.next_layer(), endpoints.begin(), endpoints.end(), ec);
            if (ec != boost::system::errc::success)
            {
                LogDebug("Connect failed: {}", ec.message());
                return;
            }

            m_ssl_socket.handshake(boost::asio::ssl::stream_base::client, ec); // NOLINT(bugprone-unused-return-value)
            if (ec != boost::system::errc::success)
            {
                LogDebug("Handshake failed: {}", ec.message());
                return;
            }
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown: {}", e.what());
            ec = boost::asio::error::operation_aborted;
        }
    }

    boost::asio::awaitable<void> HttpsSocket::AsyncConnect(
        const boost::asio::ip::tcp::resolver:: // NOLINT(cppcoreguidelines-avoid-reference-coroutine-parameters)
        results_type& endpoints,
        boost::system::error_code& ec) // NOLINT(cppcoreguidelines-avoid-reference-coroutine-parameters)
    {
        using namespace boost::asio::experimental::awaitable_operators;

        auto timer = std::make_shared<boost::asio::steady_timer>(co_await boost::asio::this_coro::executor);
        timer->expires_after(std::chrono::seconds(http_client_utils::TIMEOUT_SECONDS));

        auto result = std::make_shared<boost::system::error_code>();
        auto taskCompleted = std::make_shared<bool>(false);

        try
        {
            co_await (
                http_client_utils::TimerTask(timer, result, taskCompleted) ||
                http_client_utils::SocketConnectTask(m_ssl_socket.lowest_layer(), endpoints, result, taskCompleted));

            if (result->value() != boost::system::errc::success)
            {
                m_ssl_socket.lowest_layer().cancel();
                ec = *result;
                LogDebug("AsyncConnect error:  {}", result->value());
            }
            else
            {
                co_await m_ssl_socket.async_handshake(boost::asio::ssl::stream_base::client,
                                                      boost::asio::redirect_error(boost::asio::use_awaitable, ec));
                if (ec)
                {
                    LogDebug("Handshake failed: {}", ec.message());
                }
            }
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown: {}", e.what());
            ec = boost::asio::error::operation_aborted;
        }
    }

    void HttpsSocket::Write(const boost::beast::http::request<boost::beast::http::string_body>& req,
                            boost::system::error_code& ec)
    {
        try
        {
            boost::beast::http::write(m_ssl_socket, req, ec);
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown during write: {}", e.what());
        }
    }

    boost::asio::awaitable<void> HttpsSocket::AsyncWrite(
        const boost::beast::http::request< // NOLINT(cppcoreguidelines-avoid-reference-coroutine-parameters)
            boost::beast::http::string_body>& req,
        boost::system::error_code& ec) // NOLINT(cppcoreguidelines-avoid-reference-coroutine-parameters)
    {
        using namespace boost::asio::experimental::awaitable_operators;

        auto timer = std::make_shared<boost::asio::steady_timer>(co_await boost::asio::this_coro::executor);
        timer->expires_after(std::chrono::seconds(http_client_utils::TIMEOUT_SECONDS));

        auto result = std::make_shared<boost::system::error_code>();
        auto taskCompleted = std::make_shared<bool>(false);

        try
        {
            co_await (http_client_utils::TimerTask(timer, result, taskCompleted) ||
                      http_client_utils::SocketWriteTask(m_ssl_socket, req, result, taskCompleted));
            if (result->value() != boost::system::errc::success)
            {
                m_ssl_socket.lowest_layer().cancel();
                ec = *result;
                LogDebug("AsyncWrite error:  {}", result->value());
            }
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown during async write: {}", e.what());
            ec = boost::asio::error::operation_aborted;
        }
    }

    void HttpsSocket::Read(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                           boost::system::error_code& ec)
    {
        try
        {
            boost::beast::flat_buffer buffer;
            boost::beast::http::read(m_ssl_socket, buffer, res, ec);
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown during read: {}", e.what());
        }
    }

    boost::asio::awaitable<void> HttpsSocket::AsyncRead(
        boost::beast::http::response< // NOLINT(cppcoreguidelines-avoid-reference-coroutine-parameters)
            boost::beast::http::dynamic_body>& res,
        boost::system::error_code& ec) // NOLINT(cppcoreguidelines-avoid-reference-coroutine-parameters)
    {
        using namespace boost::asio::experimental::awaitable_operators;

        auto timer = std::make_shared<boost::asio::steady_timer>(co_await boost::asio::this_coro::executor);
        timer->expires_after(std::chrono::seconds(http_client_utils::TIMEOUT_SECONDS));

        auto result = std::make_shared<boost::system::error_code>();
        auto taskCompleted = std::make_shared<bool>(false);

        try
        {
            boost::beast::flat_buffer buffer;
            co_await (http_client_utils::TimerTask(timer, result, taskCompleted) ||
                      http_client_utils::SocketReadTask(m_ssl_socket, buffer, res, result, taskCompleted));
            if (result->value() != boost::system::errc::success)
            {
                m_ssl_socket.lowest_layer().cancel();
                ec = *result;
                LogDebug("AsyncRead error:  {}", result->value());
            }
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown during async read: {}", e.what());
            ec = boost::asio::error::operation_aborted;
        }
    }

    void HttpsSocket::Close()
    {
        try
        {
            m_ssl_socket.lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            m_ssl_socket.lowest_layer().close();
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown on socket closing: {}", e.what());
        }
    }
} // namespace http_client
