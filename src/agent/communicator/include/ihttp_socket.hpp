#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/system/error_code.hpp>

#include <string>

namespace http_client
{
    /// @brief Interface for HTTP sockets
    class IHttpSocket
    {
    public:
        /// @brief Virtual destructor
        virtual ~IHttpSocket() = default;

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
                           boost::beast::error_code& ec) = 0;

        /// @brief Asynchronous version of Write
        /// @param req The request to write
        /// @param ec The error code, if any occurred
        virtual boost::asio::awaitable<void>
        AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req,
                   boost::beast::error_code& ec) = 0;

        /// @brief Reads a response from the socket
        /// @param res The response to read
        /// @param ec The error code, if any occurred
        virtual void Read(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                          boost::beast::error_code& ec) = 0;

        /// @brief Reads a response from the socket and writes it to a file
        /// @param res The response to read
        /// @param dstFilePath The path to the file to write to
        virtual void ReadToFile(boost::beast::http::response_parser<boost::beast::http::dynamic_body>& res,
                                const std::string& dstFilePath) = 0;

        /// @brief Asynchronous version of Read
        /// @param res The response to read
        /// @param ec The error code, if any occurred
        virtual boost::asio::awaitable<void>
        AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                  boost::beast::error_code& ec) = 0;

        /// @brief Closes the socket
        virtual void Close() = 0;
    };
} // namespace http_client
