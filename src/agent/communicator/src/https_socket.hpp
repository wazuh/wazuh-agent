#pragma once

#include <https_socket_verify_utils.hpp>
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
    /// @brief Implementation of IHttpSocket for HTTPS requests
    class HttpsSocket : public IHttpSocket
    {
    public:
        /// @brief Constructor for HttpsSocket
        /// @param io_context The io context to use for the socket
        HttpsSocket(const boost::asio::any_io_executor& io_context)
            : m_ctx(boost::asio::ssl::context::sslv23)
            , m_ssl_socket(io_context, m_ctx)
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
                m_ssl_socket.set_verify_mode(boost::asio::ssl::verify_none);
                return;
            }

            m_ctx.set_default_verify_paths();
            m_ssl_socket.set_verify_mode(boost::asio::ssl::verify_peer);
            if (verificationMode == "certificate")
            {
                m_ssl_socket.set_verify_callback(
                    [verificationMode, host](bool preverified, boost::asio::ssl::verify_context& ctx)
                    { return https_socket_verify_utils::VerifyCertificate(preverified, ctx, verificationMode, host); });
            }
            else
            {
                if (verificationMode != "full")
                {
                    LogWarn("Verification mode unknown, full mode is used.");
                }
                m_ssl_socket.set_verify_callback(
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
                boost::asio::connect(m_ssl_socket.next_layer(), endpoints.begin(), endpoints.end(), ec);
                if (ec)
                {
                    LogDebug("Connect failed: {}", ec.message());
                    return;
                }

                m_ssl_socket.handshake(boost::asio::ssl::stream_base::client, ec);
                if (ec)
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

        /// @brief Asynchronous version of Connect
        /// @param endpoints The endpoints to connect to
        /// @param ec The error code, if any occurred
        boost::asio::awaitable<void> AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                  boost::system::error_code& ec) override
        {
            try
            {
                co_await boost::asio::async_connect(m_ssl_socket.lowest_layer(),
                                                    endpoints,
                                                    boost::asio::redirect_error(boost::asio::use_awaitable, ec));

                if (ec)
                {
                    LogDebug("boost::asio::async_connect returned error code: {} {}", ec.value(), ec.message());
                }

                co_await m_ssl_socket.async_handshake(boost::asio::ssl::stream_base::client,
                                                      boost::asio::redirect_error(boost::asio::use_awaitable, ec));
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
                boost::beast::http::write(m_ssl_socket, req, ec);
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
                co_await boost::beast::http::async_write(
                    m_ssl_socket, req, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
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
                boost::beast::http::read(m_ssl_socket, buffer, res, ec);
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
                co_await boost::beast::http::async_read(
                    m_ssl_socket, buffer, res, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
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
                m_ssl_socket.shutdown();
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
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_ssl_socket;
    };
} // namespace http_client
