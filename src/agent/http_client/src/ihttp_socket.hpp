#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include <boost/system/error_code.hpp>

#include <string>

namespace http_client
{
    /// @brief The socket timeout in seconds
    constexpr int SOCKET_TIMEOUT_SECS = 60;

    class ISocketHelper
    {
    public:
        virtual ~ISocketHelper() {};

        virtual void set_verify_mode(boost::asio::ssl::verify_mode mode) = 0;

        virtual void set_verify_callback(std::function<bool(bool, boost::asio::ssl::verify_context&)> vf) = 0;

        virtual void expires_after(std::chrono::seconds seconds) = 0;

        virtual void connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                             boost::system::error_code& ec) = 0;

        virtual boost::asio::awaitable<void>
        async_connect(const boost::asio::ip::tcp::resolver::results_type& endpoints, boost::system::error_code& ec) = 0;

        virtual void write(const boost::beast::http::request<boost::beast::http::string_body>& req,
                           boost::system::error_code& ec) = 0;

        virtual boost::asio::awaitable<void>
        async_write(const boost::beast::http::request<boost::beast::http::string_body>& req,
                    boost::system::error_code& ec) = 0;

        virtual void read(boost::beast::flat_buffer& buffer,
                          boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                          boost::system::error_code& ec) = 0;

        virtual boost::asio::awaitable<void>
        async_read(boost::beast::flat_buffer& buffer,
                   boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                   boost::system::error_code& ec) = 0;

        virtual void close() = 0;
    };

    /// @brief Interface for HTTP sockets
    class IHttpSocket
    {
    public:
        /// @brief Virtual destructor
        virtual ~IHttpSocket() = default;

        /// @brief Sets the verification mode for the host
        /// @param host The host name
        /// @param verificationMode The verification mode to set
        virtual void SetVerificationMode(const std::string& host, const std::string& verificationMode) = 0;

        /// @brief Connects the socket to the given endpoints
        /// @param endpoints The endpoints to connect to
        /// @param ec The error code, if any occurred
        virtual void Connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                             boost::system::error_code& ec) = 0;

        /// @brief Asynchronous version of Connect
        /// @param endpoints The endpoints to connect to
        /// @param ec The error code, if any occurred
        virtual boost::asio::awaitable<void> AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                          boost::system::error_code& ec) = 0;

        /// @brief Writes the given request to the socket
        /// @param req The request to write
        /// @param ec The error code, if any occurred
        virtual void Write(const boost::beast::http::request<boost::beast::http::string_body>& req,
                           boost::system::error_code& ec) = 0;

        /// @brief Asynchronous version of Write
        /// @param req The request to write
        /// @param ec The error code, if any occurred
        virtual boost::asio::awaitable<void>
        AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req,
                   boost::system::error_code& ec) = 0;

        /// @brief Reads a response from the socket
        /// @param res The response to read
        /// @param ec The error code, if any occurred
        virtual void Read(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                          boost::system::error_code& ec) = 0;

        /// @brief Asynchronous version of Read
        /// @param res The response to read
        /// @param ec The error code, if any occurred
        virtual boost::asio::awaitable<void>
        AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                  boost::system::error_code& ec) = 0;

        /// @brief Closes the socket
        virtual void Close() = 0;
    };
} // namespace http_client
