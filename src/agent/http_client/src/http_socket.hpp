#pragma once

#include <http_socket_wrapper.hpp>
#include <ihttp_socket.hpp>
#include <logger.hpp>

#include <boost/asio.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/system/error_code.hpp>

#include <exception>
#include <memory>
#include <string>

namespace http_client
{
    /// @brief Implementation of IHttpSocket for HTTP requests
    class HttpSocket : public IHttpSocket
    {
    public:
        /// @brief Constructor for HttpSocket
        /// @param io_context The io context to use for the socket
        /// @param socket The socket helper to use
        HttpSocket(const boost::asio::any_io_executor& io_context,
                   std::shared_ptr<http_client::ISocketWrapper> socket = nullptr)
            : m_socket(socket != nullptr ? std::move(socket)
                                         : std::make_shared<http_client::HttpSocketHelper>(io_context))
        {
        }

        /// @brief Sets the verification mode for the host
        /// @param host The host name
        /// @param verificationMode The verification mode to set
        void SetVerificationMode([[maybe_unused]] const std::string& host,
                                 [[maybe_unused]] const std::string& verificationMode) override
        {
            // No functionality for HTTP sockets
        }

        /// @brief Connects the socket to the given endpoints
        /// @param endpoints The endpoints to connect to
        /// @param ec The error code, if any occurred
        void Connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                     boost::system::error_code& ec) override
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

        /// @brief Asynchronous version of Connect
        /// @param endpoints The endpoints to connect to
        /// @param ec The error code, if any occurred
        boost::asio::awaitable<void> AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                  boost::system::error_code& ec) override
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

        /// @brief Writes the given request to the socket
        /// @param req The request to write
        /// @param ec The error code, if any occurred
        void Write(const boost::beast::http::request<boost::beast::http::string_body>& req,
                   boost::system::error_code& ec) override
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

        /// @brief Asynchronous version of Write
        /// @param req The request to write
        /// @param ec The error code, if any occurred
        boost::asio::awaitable<void> AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req,
                                                boost::system::error_code& ec) override
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

        /// @brief Reads a response from the socket
        /// @param res The response to read
        /// @param ec The error code, if any occurred
        void Read(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                  boost::system::error_code& ec) override
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

        /// @brief Asynchronous version of Read
        /// @param res The response to read
        /// @param ec The error code, if any occurred
        boost::asio::awaitable<void> AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                               boost::system::error_code& ec) override
        {
            try
            {
                m_socket->expires_after(std::chrono::seconds(http_client::SOCKET_TIMEOUT_SECS));
                boost::beast::flat_buffer buffer;
                co_await m_socket->async_read(buffer, res, ec);
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
                m_socket->close();
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown on socket closing: {}", e.what());
            }
        }

    private:
        /// @brief The socket to use for the HTTP connection
        std::shared_ptr<ISocketWrapper> m_socket;
    };
} // namespace http_client
