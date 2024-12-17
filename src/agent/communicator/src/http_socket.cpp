#include "http_socket.hpp"

#include "http_client_utils.hpp"
#include <ihttp_socket.hpp>
#include <logger.hpp>

#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/beast.hpp>
#include <boost/system/error_code.hpp>

#include <exception>
#include <string>

namespace http_client
{
    HttpSocket::HttpSocket(const boost::asio::any_io_executor& ioContext)
        : m_socket(ioContext)
    {
    }

    void HttpSocket::Connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                             boost::system::error_code& ec)
    {
        try
        {
            boost::asio::connect(m_socket, endpoints, ec);
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown: {}", e.what());
        }
    }

    boost::asio::awaitable<void> HttpSocket::AsyncConnect(
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
            co_await (http_client_utils::TimerTask(timer, result, taskCompleted) ||
                      http_client_utils::SocketConnectTask(m_socket, endpoints, result, taskCompleted));
            if (!result)
            {
                m_socket.cancel();
                ec = *result;
                LogDebug("AsyncConnect error:  {}", result->value());
            }
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown during async connect: {}", e.what());
            ec = boost::asio::error::operation_aborted;
        }
    }

    void HttpSocket::Write(const boost::beast::http::request<boost::beast::http::string_body>& req,
                           boost::system::error_code& ec)
    {
        try
        {
            boost::beast::http::write(m_socket, req, ec);
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown during write: {}", e.what());
        }
    }

    boost::asio::awaitable<void> HttpSocket::AsyncWrite(
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
                      http_client_utils::SocketWriteTask(m_socket, req, result, taskCompleted));
            if (!result)
            {
                m_socket.cancel();
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

    void HttpSocket::Read(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                          boost::system::error_code& ec)
    {
        try
        {
            boost::beast::flat_buffer buffer;
            boost::beast::http::read(m_socket, buffer, res, ec);
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown during read: {}", e.what());
        }
    }

    void HttpSocket::ReadToFile(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                const std::string& dstFilePath)
    {
        try
        {
            http_client_utils::ReadToFile(m_socket, res, dstFilePath);
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown during read to file: {}", e.what());
        }
    }

    boost::asio::awaitable<void> HttpSocket::AsyncRead(
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
                      http_client_utils::SocketReadTask(m_socket, buffer, res, result, taskCompleted));
            if (!result)
            {
                m_socket.cancel();
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

    void HttpSocket::Close()
    {
        try
        {
            m_socket.close();
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown on socket closing: {}", e.what());
        }
    }
} // namespace http_client
