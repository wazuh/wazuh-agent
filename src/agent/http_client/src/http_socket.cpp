#include <http_socket.hpp>

#include <logger.hpp>

#include <exception>

// NOLINTBEGIN (cppcoreguidelines-avoid-reference-coroutine-parameters)
namespace http_client
{
    HttpSocket::HttpSocket(const boost::asio::any_io_executor& ioContext,
                           std::shared_ptr<http_client::ISocketWrapper> socket)
        : m_socket(socket != nullptr ? std::move(socket) : std::make_shared<http_client::HttpSocketHelper>(ioContext))
    {
    }

    void HttpSocket::SetVerificationMode([[maybe_unused]] const std::string& host,
                                         [[maybe_unused]] const std::string& verificationMode)
    {
        // No functionality for HTTP sockets
    }

    void HttpSocket::Connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                             boost::system::error_code& ec)
    {
        try
        {
            m_socket->expires_after(std::chrono::seconds(http_client::SOCKET_TIMEOUT_SECS));
            m_socket->connect(endpoints, ec);
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown: {}", e.what());
            ec = boost::asio::error::operation_aborted;
        }
    }

    boost::asio::awaitable<void> HttpSocket::AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                          boost::system::error_code& ec)
    {
        try
        {
            m_socket->expires_after(std::chrono::seconds(http_client::SOCKET_TIMEOUT_SECS));
            co_await m_socket->async_connect(endpoints, ec);
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
            m_socket->expires_after(std::chrono::seconds(http_client::SOCKET_TIMEOUT_SECS));
            m_socket->write(req, ec);
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown during write: {}", e.what());
        }
    }

    boost::asio::awaitable<void>
    HttpSocket::AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req,
                           boost::system::error_code& ec)
    {
        try
        {
            m_socket->expires_after(std::chrono::seconds(http_client::SOCKET_TIMEOUT_SECS));
            co_await m_socket->async_write(req, ec);
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
            m_socket->expires_after(std::chrono::seconds(http_client::SOCKET_TIMEOUT_SECS));
            boost::beast::flat_buffer buffer;
            m_socket->read(buffer, res, ec);
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown during read: {}", e.what());
        }
    }

    boost::asio::awaitable<void>
    HttpSocket::AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                          boost::system::error_code& ec,
                          const std::time_t socketTimeout)
    {
        try
        {
            m_socket->expires_after(std::chrono::milliseconds(socketTimeout));
            boost::beast::flat_buffer buffer;
            co_await m_socket->async_read(buffer, res, ec);
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
            m_socket->close();
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown on socket closing: {}", e.what());
        }
    }
} // namespace http_client

// NOLINTEND (cppcoreguidelines-avoid-reference-coroutine-parameters)
