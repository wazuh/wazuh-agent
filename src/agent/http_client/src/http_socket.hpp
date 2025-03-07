#pragma once

#include <http_socket_wrapper.hpp>
#include <ihttp_socket.hpp>

#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <boost/system/error_code.hpp>

#include <memory>
#include <string>

namespace http_client
{
    /// @brief Implementation of IHttpSocket for HTTP requests
    class HttpSocket : public IHttpSocket
    {
    public:
        /// @brief Constructor for HttpSocket
        /// @param ioContext The io context to use for the socket
        /// @param socket The socket helper to use
        HttpSocket(const boost::asio::any_io_executor& ioContext,
                   std::shared_ptr<http_client::ISocketWrapper> socket = nullptr);

        /// @copydoc IHttpSocket::SetVerificationMode
        void SetVerificationMode(const std::string& host, const std::string& verificationMode) override;

        /// @copydoc IHttpSocket::SetTimeout
        void SetTimeout(const std::chrono::milliseconds timeout) override;

        /// @copydoc IHttpSocket::Connect
        void Connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                     boost::system::error_code& ec) override;

        /// @copydoc IHttpSocket::AsyncConnect
        boost::asio::awaitable<void> AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                  boost::system::error_code& ec) override;

        /// @copydoc IHttpSocket::Write
        void Write(const boost::beast::http::request<boost::beast::http::string_body>& req,
                   boost::system::error_code& ec) override;

        /// @copydoc IHttpSocket::AsyncWrite
        boost::asio::awaitable<void> AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req,
                                                boost::system::error_code& ec) override;

        /// @copydoc IHttpSocket::Read
        void Read(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                  boost::system::error_code& ec) override;

        /// @copydoc IHttpSocket::AsyncRead
        boost::asio::awaitable<void> AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                               boost::system::error_code& ec) override;

        /// @copydoc IHttpSocket::Close
        void Close() override;

    private:
        /// @brief The socket to use for the HTTP connection
        std::shared_ptr<ISocketWrapper> m_socket;

        /// @brief Timeout in milliseconds used in every operation
        std::chrono::milliseconds m_timeout {http_client::SOCKET_TIMEOUT};
    };
} // namespace http_client
