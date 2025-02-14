#include <https_socket.hpp>

#include <logger.hpp>

#include <exception>

// NOLINTBEGIN (cppcoreguidelines-avoid-reference-coroutine-parameters)
namespace http_client
{
    HttpsSocket::HttpsSocket(const boost::asio::any_io_executor& ioContext,
                             std::shared_ptr<http_client::ISocketWrapper> socket)
        : m_ctx(boost::asio::ssl::context::sslv23)
        , m_ssl_socket(socket != nullptr ? std::move(socket)
                                         : std::make_shared<http_client::HttpsSocketHelper>(ioContext, m_ctx))
    {
    }

    void HttpsSocket::SetVerificationMode(const std::string& host, const std::string& verificationMode)
    {
        if (verificationMode == "none")
        {
            m_ssl_socket->set_verify_mode(boost::asio::ssl::verify_none);
            return;
        }

        m_ctx.set_default_verify_paths();
        m_ssl_socket->set_verify_mode(boost::asio::ssl::verify_peer);
        if (verificationMode == "certificate")
        {
            m_ssl_socket->set_verify_callback(
                [verificationMode, host](bool preverified, boost::asio::ssl::verify_context& ctx)
                { return https_socket_verify_utils::VerifyCertificate(preverified, ctx, verificationMode, host); });
        }
        else
        {
            if (verificationMode != "full")
            {
                LogWarn("Verification mode unknown, full mode is used.");
            }
            m_ssl_socket->set_verify_callback(
                [verificationMode, host](bool preverified, boost::asio::ssl::verify_context& ctx)
                { return https_socket_verify_utils::VerifyCertificate(preverified, ctx, "full", host); });
        }
    }

    void HttpsSocket::Connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                              boost::system::error_code& ec)
    {
        try
        {
            m_ssl_socket->expires_after(std::chrono::milliseconds(http_client::SOCKET_TIMEOUT_MSECS));
            m_ssl_socket->connect(endpoints, ec);
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown: {}", e.what());
            ec = boost::asio::error::operation_aborted;
        }
    }

    boost::asio::awaitable<void>
    HttpsSocket::AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                              boost::system::error_code& ec)
    {
        try
        {
            m_ssl_socket->expires_after(std::chrono::milliseconds(http_client::SOCKET_TIMEOUT_MSECS));
            co_await m_ssl_socket->async_connect(endpoints, ec);
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
            m_ssl_socket->expires_after(std::chrono::milliseconds(http_client::SOCKET_TIMEOUT_MSECS));
            m_ssl_socket->write(req, ec);
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown during write: {}", e.what());
        }
    }

    boost::asio::awaitable<void>
    HttpsSocket::AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req,
                            boost::system::error_code& ec)
    {
        try
        {
            m_ssl_socket->expires_after(std::chrono::milliseconds(http_client::SOCKET_TIMEOUT_MSECS));
            co_await m_ssl_socket->async_write(req, ec);
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
            m_ssl_socket->expires_after(std::chrono::milliseconds(http_client::SOCKET_TIMEOUT_MSECS));
            m_ssl_socket->read(buffer, res, ec);
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown during read: {}", e.what());
        }
    }

    boost::asio::awaitable<void>
    HttpsSocket::AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                           boost::system::error_code& ec,
                           const time_t socketTimeout)
    {
        try
        {
            boost::beast::flat_buffer buffer;
            m_ssl_socket->expires_after(std::chrono::milliseconds(socketTimeout));
            co_await m_ssl_socket->async_read(buffer, res, ec);
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
            m_ssl_socket->close();
        }
        catch (const std::exception& e)
        {
            LogDebug("Exception thrown on socket closing: {}", e.what());
        }
    }

} // namespace http_client

// NOLINTEND (cppcoreguidelines-avoid-reference-coroutine-parameters)
