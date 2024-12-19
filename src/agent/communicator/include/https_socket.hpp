#pragma once

#include <ihttp_socket.hpp>

#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/system/error_code.hpp>

#include <string>

namespace http_client
{
    /// @brief Implementation of IHttpSocket for HTTPS requests
    class HttpsSocket : public IHttpSocket
    {
    public:
        /// @brief Constructor for HttpsSocket
        /// @param ioContext The io context to use for the socket
        HttpsSocket(const boost::asio::any_io_executor& ioContext)
            : m_ctx(boost::asio::ssl::context::sslv23)
            , m_ssl_socket(ioContext, m_ctx)
        {
            m_ctx.set_verify_mode(boost::asio::ssl::verify_peer);
        }

        /// @brief Connects the socket to the given endpoints
        /// @param endpoints The endpoints to connect to
        /// @param ec The error code, if any occurred
        /// @param timeOut The timeout for the connection
        void Connect(boost::asio::io_context& ioContext,
                     const boost::asio::ip::tcp::resolver::results_type& endpoints,
                     boost::system::error_code& ec,
                     const std::chrono::seconds timeOut = std::chrono::seconds(TIMEOUT_DEFAULT)) override;

        /// @brief Asynchronous version of Connect
        /// @param endpoints The endpoints to connect to
        /// @param ec The error code, if any occurred
        /// @param timeOut The timeout for the connection
        boost::asio::awaitable<void>
        AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                     boost::system::error_code& ec,
                     const std::chrono::seconds timeOut = std::chrono::seconds(TIMEOUT_DEFAULT)) override;

        /// @brief Writes the given request to the socket
        /// @param ioContext The io context associated to the socket
        /// @param req The request to write
        /// @param ec The error code, if any occurred
        /// @param timeOut The timeout for the write
        void Write(boost::asio::io_context& ioContext,
                   const boost::beast::http::request<boost::beast::http::string_body>& req,
                   boost::system::error_code& ec,
                   const std::chrono::seconds timeOut = std::chrono::seconds(TIMEOUT_DEFAULT)) override;

        /// @brief Asynchronous version of Write
        /// @param req The request to write
        /// @param ec The error code, if any occurred
        /// @param timeOut The timeout for the write
        boost::asio::awaitable<void>
        AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req,
                   boost::system::error_code& ec,
                   const std::chrono::seconds timeOut = std::chrono::seconds(TIMEOUT_DEFAULT)) override;

        /// @brief Reads a response from the socket
        /// @param ioContext The io context associated to the socket
        /// @param res The response
        /// @param ec The error code, if any occurred
        /// @param timeOut The timeout for the read
        void Read(boost::asio::io_context& ioContext,
                  boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                  boost::system::error_code& ec,
                  const std::chrono::seconds timeOut = std::chrono::seconds(TIMEOUT_DEFAULT)) override;

        /// @brief Reads a response from the socket and writes it to a file
        /// @param res The response to read
        /// @param dstFilePath The path to the file to write to
        void ReadToFile(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                        const std::string& dstFilePath) override;

        /// @brief Asynchronous version of Read
        /// @param res The response to read
        /// @param ec The error code, if any occurred
        /// @param timeOut The timeout for the read
        boost::asio::awaitable<void>
        AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                  boost::system::error_code& ec,
                  const std::chrono::seconds timeOut = std::chrono::seconds(TIMEOUT_DEFAULT)) override;

        /// @brief Closes the socket
        void Close() override;

    private:
        /// @brief The SSL context to use for the socket
        boost::asio::ssl::context m_ctx;

        /// @brief The SSL socket to use for the connection
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_ssl_socket;
    };
} // namespace http_client
