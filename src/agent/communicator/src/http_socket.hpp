#pragma once

#include <ihttp_socket.hpp>
#include <logger.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/system/error_code.hpp>

#include <exception>
#include <string>

namespace http_client
{
    /// @brief Implementation of IHttpSocket for HTTP requests
    class HttpSocket : public IHttpSocket
    {
    public:
        /// @brief Constructor for HttpSocket
        /// @param io_context The io context to use for the socket
        HttpSocket(const boost::asio::any_io_executor& io_context)
            : m_socket(io_context)
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
                boost::asio::connect(m_socket, endpoints, ec);
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown: {}", e.what());
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
                co_await boost::asio::async_connect(
                    m_socket, endpoints, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
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
                boost::beast::http::write(m_socket, req, ec);
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
                    m_socket, req, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
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
                boost::beast::http::read(m_socket, buffer, res, ec);
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
                    m_socket, buffer, res, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
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
                m_socket.close();
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown on socket closing: {}", e.what());
            }
        }

    private:
        /// @brief The socket to use for the HTTP connection
        boost::asio::ip::tcp::socket m_socket;
    };
} // namespace http_client
