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
    void HttpSocket::Connect(boost::asio::io_context& ioContext,
                             const boost::asio::ip::tcp::resolver::results_type& endpoints,
                             boost::system::error_code& ec,
                             const std::chrono::seconds timeOut)
    {
        try
        {
            bool connectionSuccess = false;

            boost::asio::async_connect(m_socket,
                                       endpoints,
                                       [&](const boost::system::error_code& errorCode, const auto&)
                                       {
                                           if (!errorCode)
                                           {
                                               connectionSuccess = true;
                                               ec = errorCode;
                                           }
                                       });

            ioContext.run_for(timeOut); // Run for 2 seconds();
            if (!connectionSuccess)
            {
                ec = boost::asio::error::timed_out;
                LogDebug("Connection timed out");
            }
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown: {}", e.what());
        }
    }

    boost::asio::awaitable<void>
    HttpSocket::AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints, // NOLINT
                             boost::system::error_code& ec,                                 // NOLINT
                             const std::chrono::seconds timeOut)
    {
        using namespace boost::asio::experimental::awaitable_operators;

        auto timer = std::make_shared<boost::asio::steady_timer>(co_await boost::asio::this_coro::executor);
        timer->expires_after(timeOut);

        auto result = std::make_shared<boost::system::error_code>();
        auto taskCompleted = std::make_shared<bool>(false);

        try
        {
            co_await (http_client_utils::TimerTask(timer, result, taskCompleted) ||
                      http_client_utils::SocketConnectTask(m_socket, endpoints, result, taskCompleted));
            if (!result)
            {
                LogDebug("Connection error:  {}", result->value());
            }
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown during async connect: {}", e.what());
            ec = boost::asio::error::operation_aborted;
        }
    }

    void HttpSocket::Write(boost::asio::io_context& ioContext,
                           const boost::beast::http::request<boost::beast::http::string_body>& req,
                           boost::system::error_code& ec,
                           const std::chrono::seconds timeOut)
    {
        ioContext.reset();
        try
        {
            bool writeSuccess = false;
            boost::beast::http::async_write(m_socket,
                                            req,
                                            [&](const boost::system::error_code& errorCode, std::size_t)
                                            {
                                                if (!errorCode)
                                                {
                                                    writeSuccess = true;
                                                    ec = errorCode;
                                                }
                                                else
                                                {
                                                    LogDebug("Write error: {}", errorCode.message());
                                                }
                                            });

            ioContext.run_for(timeOut);
            if (!writeSuccess)
            {
                ec = boost::asio::error::timed_out;
                LogDebug("Write operation timed out");
            }
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown during write: {}", e.what());
        }
    }

    boost::asio::awaitable<void>
    HttpSocket::AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req, // NOLINT
                           boost::system::error_code& ec,                                           // NOLINT
                           const std::chrono::seconds timeOut)
    {
        using namespace boost::asio::experimental::awaitable_operators;

        auto timer = std::make_shared<boost::asio::steady_timer>(co_await boost::asio::this_coro::executor);
        timer->expires_after(timeOut);

        auto result = std::make_shared<boost::system::error_code>();
        auto taskCompleted = std::make_shared<bool>(false);

        try
        {
            co_await (http_client_utils::TimerTask(timer, result, taskCompleted) ||
                      http_client_utils::SocketWriteTask(m_socket, req, result, taskCompleted));
            if (!result)
            {
                LogDebug("Write error:  {}", result->value());
            }
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown during async write: {}", e.what());
            ec = boost::asio::error::operation_aborted;
        }
    }

    void HttpSocket::Read(boost::asio::io_context& ioContext,
                          boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                          boost::system::error_code& ec,
                          const std::chrono::seconds timeOut)
    {
        ioContext.reset();
        try
        {
            bool readSuccess = false;

            boost::beast::flat_buffer buffer;
            boost::beast::http::async_read(m_socket,
                                           buffer,
                                           res,
                                           [&](const boost::system::error_code& errorCode, std::size_t)
                                           {
                                               if (!errorCode)
                                               {
                                                   readSuccess = true;
                                                   ec = errorCode;
                                               }
                                               else
                                               {
                                                   LogDebug("Read error: {}", errorCode.message());
                                               }
                                           });

            ioContext.run_for(timeOut);
            if (!readSuccess)
            {
                ec = boost::asio::error::timed_out;
                LogDebug("Read operation timed out");
            }
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

    boost::asio::awaitable<void>
    HttpSocket::AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res, // NOLINT
                          boost::system::error_code& ec,                                       // NOLINT
                          const std::chrono::seconds timeOut)
    {
        using namespace boost::asio::experimental::awaitable_operators;

        auto timer = std::make_shared<boost::asio::steady_timer>(co_await boost::asio::this_coro::executor);
        timer->expires_after(timeOut);

        auto result = std::make_shared<boost::system::error_code>();
        auto taskCompleted = std::make_shared<bool>(false);

        try
        {
            boost::beast::flat_buffer buffer;
            co_await (http_client_utils::TimerTask(timer, result, taskCompleted) ||
                      http_client_utils::SocketReadTask(m_socket, buffer, res, result, taskCompleted));
            if (!result)
            {
                LogDebug("Write error:  {}", result->value());
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
