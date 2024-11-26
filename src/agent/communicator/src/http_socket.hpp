#include "http_client_utils.hpp"
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

        /// @brief Connects the socket to the given endpoints
        /// @param endpoints The endpoints to connect to
        void Connect(const boost::asio::ip::tcp::resolver::results_type& endpoints) override
        {
            boost::asio::connect(m_socket, endpoints);
        }

        /// @brief Asynchronous version of Connect
        /// @param endpoints The endpoints to connect to
        /// @param code The error code to store the result in
        boost::asio::awaitable<void> AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                  boost::system::error_code& code) override
        {
            co_await boost::asio::async_connect(
                m_socket, endpoints, boost::asio::redirect_error(boost::asio::use_awaitable, code));
        }

        /// @brief Writes the given request to the socket
        /// @param req The request to write
        void Write(const boost::beast::http::request<boost::beast::http::string_body>& req) override
        {
            boost::beast::http::write(m_socket, req);
        }

        /// @brief Asynchronous version of Write
        /// @param req The request to write
        /// @param ec The error code to store the result in
        boost::asio::awaitable<void> AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req,
                                                boost::beast::error_code& ec) override
        {
            co_await boost::beast::http::async_write(
                m_socket, req, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        }

        /// @brief Reads a response from the socket
        /// @param res The response to read
        void Read(boost::beast::http::response<boost::beast::http::dynamic_body>& res) override
        {
            boost::beast::flat_buffer buffer;
            boost::beast::http::read(m_socket, buffer, res);
        }

        /// @brief Reads a response from the socket and writes it to a file
        /// @param res The response to read
        /// @param dstFilePath The path to the file to write to
        void ReadToFile(boost::beast::http::response_parser<boost::beast::http::dynamic_body>& res,
                        const std::string& dstFilePath) override
        {
            http_client_utils::ReadToFile(m_socket, res, dstFilePath);
        }

        /// @brief Asynchronous version of Read
        /// @param res The response to read
        /// @param ec The error code to store the result in
        boost::asio::awaitable<void> AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                               boost::beast::error_code& ec) override
        {
            boost::beast::flat_buffer buffer;
            co_await boost::beast::http::async_read(
                m_socket, buffer, res, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
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
                LogError("Exception thrown on socket closing: {}", e.what());
            }
        }

    private:
        /// @brief The socket to use for the HTTP connection
        boost::asio::ip::tcp::socket m_socket;
    };
} // namespace http_client
