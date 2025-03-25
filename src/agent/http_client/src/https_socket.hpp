#pragma once

#include <https_socket_verify_utils.hpp>
#include <https_socket_wrapper.hpp>
#include <ihttp_socket.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/http.hpp>
#include <boost/system/error_code.hpp>

#include <memory>
#include <string>

namespace http_client
{
    /// @brief Implementation of IHttpSocket for HTTPS requests
    class HttpsSocket : public IHttpSocket
    {
    public:
        /// @brief Constructor for HttpsSocket
        /// @param ioContext The io context to use for the socket
        /// @param socket The socket helper to use
        HttpsSocket(const boost::asio::any_io_executor& ioContext,
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

        /// @copydoc IHttpSocket::Shutdown
        void Shutdown(boost::system::error_code& ec) override;

    private:
        /// @brief The SSL context to use for the socket
        boost::asio::ssl::context m_ctx;

        /// @brief The SSL socket to use for the connection
        std::shared_ptr<ISocketWrapper> m_ssl_socket;

        /// @brief Timeout in milliseconds used in every operation
        std::chrono::milliseconds m_timeout {http_client::SOCKET_TIMEOUT};
    };
} // namespace http_client
