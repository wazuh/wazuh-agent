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

        /// @brief Sets the verification mode for the host
        /// @param host The host name
        /// @param verificationMode The verification mode to set
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
        /// @param socketTimeout Timeout for reading in millisenconds
        boost::asio::awaitable<void> AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                               boost::system::error_code& ec,
                                               const time_t socketTimeout = http_client::SOCKET_TIMEOUT_MSECS) override;

        /// @brief Closes the socket
        void Close() override;

    private:
        /// @brief The socket to use for the HTTP connection
        std::shared_ptr<ISocketWrapper> m_socket;
    };
} // namespace http_client
