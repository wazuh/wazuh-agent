#pragma once

#include <https_socket_verify_utils.hpp>
#include <ihttp_socket.hpp>
#include <logger.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/system/error_code.hpp>

#include <exception>
#include <string>

namespace http_client
{
    /// @brief Helper class that wraps boost network functions for testing purposes
    class HttpsSocketHelper : public ISocketHelper
    {
    public:
        HttpsSocketHelper(const boost::asio::any_io_executor& io_context, boost::asio::ssl::context& ctx)
            : m_socket(io_context, ctx)
        {
        }

        virtual ~HttpsSocketHelper() {}

        void set_verify_mode(boost::asio::ssl::verify_mode mode) override
        {
            m_socket.set_verify_mode(mode);
        }

        void set_verify_callback(std::function<bool(bool, boost::asio::ssl::verify_context&)> vf) override
        {
            m_socket.set_verify_callback(vf);
        }

        void expires_after(std::chrono::seconds seconds) override
        {
            m_socket.next_layer().expires_after(seconds);
        }

        void connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                     boost::system::error_code& ec) override
        {
            m_socket.next_layer().async_connect(
                endpoints,
                [this, &ec](boost::system::error_code const& ecConnect, auto const&)
                {
                    ec = ecConnect;
                    if (ec)
                    {
                        LogDebug("Connect failed: {}", ec.message());
                        return;
                    }

                    m_socket.next_layer().expires_after(std::chrono::seconds(http_client::SOCKET_TIMEOUT_SECS));
                    m_socket.async_handshake(boost::asio::ssl::stream_base::client,
                                             [&ec](const boost::system::error_code& ecHandshake)
                                             {
                                                 ec = ecHandshake;
                                                 if (ecHandshake)
                                                 {
                                                     LogInfo("Handshake failed: {}", ecHandshake.message());
                                                 }
                                             });
                });
        }

        boost::asio::awaitable<void> async_connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                   boost::system::error_code& ec) override
        {
            co_await m_socket.next_layer().async_connect(endpoints,
                                                         boost::asio::redirect_error(boost::asio::use_awaitable, ec));

            if (ec)
            {
                LogDebug("boost::asio::async_connect returned error code: {} {}", ec.value(), ec.message());
            }
            else
            {
                co_await m_socket.async_handshake(boost::asio::ssl::stream_base::client,
                                                  boost::asio::redirect_error(boost::asio::use_awaitable, ec));
            }
        }

        void write(const boost::beast::http::request<boost::beast::http::string_body>& req,
                   boost::system::error_code& ec) override
        {
            boost::beast::http::write(m_socket, req, ec);
        }

        boost::asio::awaitable<void>
        async_write(const boost::beast::http::request<boost::beast::http::string_body>& req,
                    boost::system::error_code& ec) override
        {
            co_await boost::beast::http::async_write(
                m_socket, req, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        }

        void read(boost::beast::flat_buffer& buffer,
                  boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                  boost::system::error_code& ec) override
        {
            boost::beast::http::read(m_socket, buffer, res, ec);
        }

        boost::asio::awaitable<void> async_read(boost::beast::flat_buffer& buffer,
                                                boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                                boost::system::error_code& ec) override
        {
            co_await boost::beast::http::async_read(
                m_socket, buffer, res, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        }

        void close() override
        {
            m_socket.shutdown();
        }

        bool is_open() override
        {
            return m_socket.next_layer().socket().is_open();
        }

    private:
        boost::beast::ssl_stream<boost::beast::tcp_stream> m_socket;
    };

    /// @brief Implementation of IHttpSocket for HTTPS requests
    class HttpsSocket : public IHttpSocket
    {
    public:
        /// @brief Constructor for HttpsSocket
        /// @param io_context The io context to use for the socket
        /// @param socket The socket helper to use
        HttpsSocket(const boost::asio::any_io_executor& io_context,
                    std::shared_ptr<http_client::ISocketHelper> socket = nullptr)
            : m_ctx(boost::asio::ssl::context::sslv23)
            , m_ssl_socket(socket != nullptr ? std::move(socket)
                                             : std::make_shared<http_client::HttpsSocketHelper>(io_context, m_ctx))
        {
        }

        /// @brief Set the verification mode of the HTTPS connection
        /// @param host The host name to be verified
        /// @param verificationMode The verification mode to use
        /// @details The verification modes supported are:
        /// - "full": verifies the identity of the server, including the hostname
        /// - "certificate": only verifies the identity of the server, without checking the hostname
        /// - "none": no verification is performed
        void SetVerificationMode(const std::string& host, const std::string& verificationMode) override
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

        /// @brief Connects the socket to the given endpoints
        /// @param endpoints The endpoints to connect to
        /// @param ec The error code, if any occurred
        void Connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                     boost::system::error_code& ec) override
        {
            try
            {
                m_ssl_socket->expires_after(std::chrono::seconds(http_client::SOCKET_TIMEOUT_SECS));
                m_ssl_socket->connect(endpoints, ec);
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown: {}", e.what());
                ec = boost::asio::error::operation_aborted;
            }
        }

        /// @brief Asynchronous version of Connect
        /// @param endpoints The endpoints to connect to
        /// @param ec The error code, if any occurred
        boost::asio::awaitable<void> AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                  boost::system::error_code& ec) override
        {
            try
            {
                m_ssl_socket->expires_after(std::chrono::seconds(http_client::SOCKET_TIMEOUT_SECS));
                co_await m_ssl_socket->async_connect(endpoints, ec);
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown: {}", e.what());
                ec = boost::asio::error::operation_aborted;
            }
        }

        /// @brief Writes the given request to the socket
        /// @param req The request to write
        /// @param ec The error code, if any occurred
        void Write(const boost::beast::http::request<boost::beast::http::string_body>& req,
                   boost::system::error_code& ec) override
        {
            try
            {
                m_ssl_socket->expires_after(std::chrono::seconds(http_client::SOCKET_TIMEOUT_SECS));
                m_ssl_socket->write(req, ec);
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown during write: {}", e.what());
            }
        }

        /// @brief Asynchronous version of Write
        /// @param req The request to write
        /// @param ec The error code, if any occurred
        boost::asio::awaitable<void> AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req,
                                                boost::system::error_code& ec) override
        {
            try
            {
                m_ssl_socket->expires_after(std::chrono::seconds(http_client::SOCKET_TIMEOUT_SECS));
                co_await m_ssl_socket->async_write(req, ec);
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown during async write: {}", e.what());
                ec = boost::asio::error::operation_aborted;
            }
        }

        /// @brief Reads a response from the socket
        /// @param res The response to read
        /// @param ec The error code, if any occurred
        void Read(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                  boost::system::error_code& ec) override
        {
            try
            {
                boost::beast::flat_buffer buffer;
                m_ssl_socket->expires_after(std::chrono::seconds(http_client::SOCKET_TIMEOUT_SECS));
                m_ssl_socket->read(buffer, res, ec);
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown during read: {}", e.what());
            }
        }

        /// @brief Asynchronous version of Read
        /// @param res The response to read
        /// @param ec The error code, if any occurred
        boost::asio::awaitable<void> AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                               boost::system::error_code& ec) override
        {
            try
            {
                boost::beast::flat_buffer buffer;
                m_ssl_socket->expires_after(std::chrono::seconds(http_client::SOCKET_TIMEOUT_SECS));
                co_await m_ssl_socket->async_read(buffer, res, ec);
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown during async read: {}", e.what());
                ec = boost::asio::error::operation_aborted;
            }
        }

        /// @brief Closes the socket
        void Close() override
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

    private:
        /// @brief The SSL context to use for the socket
        boost::asio::ssl::context m_ctx;

        /// @brief The SSL socket to use for the connection
        std::shared_ptr<ISocketHelper> m_ssl_socket;
    };
} // namespace http_client
