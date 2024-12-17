#pragma once

#include <ihttp_socket.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/system/error_code.hpp>

#include <string>

namespace http_client
{
    /// @brief Implementation of IHttpSocket for HTTP requests
    class HttpSocket : public IHttpSocket
    {
    public:
        /// @brief Constructor for HttpSocket
        /// @param io_context The io context to use for the socket
        HttpSocket(const boost::asio::any_io_executor& ioContext);

        /// @brief Connects the socket to the given endpoints
        /// @param io_context The io_context associated to the socket
        /// @param endpoints The endpoints to connect to
        /// @param ec The error code, if any occurred
        void Connect(boost::asio::io_context& ioContext,
                     const boost::asio::ip::tcp::resolver::results_type& endpoints,
                     boost::system::error_code& ec) override;

        /// @brief Asynchronous version of Connect
        /// @param endpoints The endpoints to connect to
        /// @param ec The error code, if any occurred
        boost::asio::awaitable<void> AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                  boost::system::error_code& ec) override;

        /// @brief Writes the given request to the socket
        /// @param io_context The io context associated to the socket
        /// @param req The request to write
        /// @param ec The error code, if any occurred
        void Write(boost::asio::io_context& ioContext,
                   const boost::beast::http::request<boost::beast::http::string_body>& req,
                   boost::system::error_code& ec) override;

        /// @brief Asynchronous version of Write
        /// @param req The request to write
        /// @param ec The error code, if any occurred
        boost::asio::awaitable<void> AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req,
                                                boost::system::error_code& ec) override;

        /// @brief Reads a response from the socket
        /// @param io_context The io context associated to the socket
        /// @param res The response to read
        /// @param ec The error code, if any occurred
        void Read(boost::asio::io_context& ioContext,
                  boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                  boost::system::error_code& ec) override;

        /// @brief Reads a response from the socket and writes it to a file
        /// @param res The response to read
        /// @param dstFilePath The path to the file to write to
        void ReadToFile(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                        const std::string& dstFilePath) override;

        /// @brief Asynchronous version of Read
        /// @param res The response to read
        /// @param ec The error code, if any occurred
        boost::asio::awaitable<void> AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                               boost::system::error_code& ec) override;

        /// @brief Closes the socket
        void Close() override;

    private:
        /// @brief The socket to use for the HTTP connection
        boost::asio::ip::tcp::socket m_socket;
    };
} // namespace http_client
