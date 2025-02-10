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

        /// @brief Set the verification mode of the HTTPS connection
        /// @param host The host name to be verified
        /// @param verificationMode The verification mode to use
        /// @details The verification modes supported are:
        /// - "full": verifies the identity of the server, including the hostname
        /// - "certificate": only verifies the identity of the server, without checking the hostname
        /// - "none": no verification is performed
        void SetVerificationMode(const std::string& host, const std::string& verificationMode) override;

        /// @brief Connects the socket to the given endpoints
        /// @param endpoints The endpoints to connect to
        /// @param ec The error code, if any occurred
        void Connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                     boost::system::error_code& ec) override;

        /// @brief Asynchronous version of Connect
        /// @param endpoints The endpoints to connect to
        /// @param ec The error code, if any occurred
        boost::asio::awaitable<void> AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                  boost::system::error_code& ec) override;

        /// @brief Writes the given request to the socket
        /// @param req The request to write
        /// @param ec The error code, if any occurred
        void Write(const boost::beast::http::request<boost::beast::http::string_body>& req,
                   boost::system::error_code& ec) override;

        /// @brief Asynchronous version of Write
        /// @param req The request to write
        /// @param ec The error code, if any occurred
        boost::asio::awaitable<void> AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req,
                                                boost::system::error_code& ec) override;

        /// @brief Reads a response from the socket
        /// @param res The response to read
        /// @param ec The error code, if any occurred
        void Read(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                  boost::system::error_code& ec) override;

        /// @brief Asynchronous version of Read
        /// @param res The response to read
        /// @param ec The error code, if any occurred
        boost::asio::awaitable<void> AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                               boost::system::error_code& ec) override;

        /// @brief Closes the socket
        void Close() override;

    private:
        /// @brief The SSL context to use for the socket
        boost::asio::ssl::context m_ctx;

        /// @brief The SSL socket to use for the connection
        std::shared_ptr<ISocketWrapper> m_ssl_socket;
    };
} // namespace http_client
