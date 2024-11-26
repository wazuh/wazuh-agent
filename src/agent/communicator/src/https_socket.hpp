#include <ihttp_socket.hpp>
#include <logger.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/system/error_code.hpp>

#include <exception>
#include <string>

namespace http_client
{
    /// @brief Implementation of IHttpSocket for HTTPS requests
    class HttpsSocket : public IHttpSocket
    {
    public:
        /// @brief Constructor for HttpsSocket
        /// @param io_context The io context to use for the socket
        HttpsSocket(const boost::asio::any_io_executor& io_context)
            : m_ctx(boost::asio::ssl::context::sslv23)
            , m_ssl_socket(io_context, m_ctx)
        {
            m_ctx.set_verify_mode(boost::asio::ssl::verify_peer);
        }

        /// @brief Connects the socket to the given endpoints
        /// @param endpoints The endpoints to connect to
        void Connect(const boost::asio::ip::tcp::resolver::results_type& endpoints) override
        {
            try
            {
                boost::asio::connect(m_ssl_socket.next_layer(), endpoints.begin(), endpoints.end());
                m_ssl_socket.handshake(boost::asio::ssl::stream_base::client);
            }
            catch (const std::exception& e)
            {
                LogError("Exception thrown: {}", e.what());
            }
        }

        /// @brief Asynchronous version of Connect
        /// @param endpoints The endpoints to connect to
        /// @param code The error code to store the result in
        boost::asio::awaitable<void> AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                  boost::system::error_code& code) override
        {
            try
            {
                co_await boost::asio::async_connect(m_ssl_socket.lowest_layer(),
                                                    endpoints,
                                                    boost::asio::redirect_error(boost::asio::use_awaitable, code));

                if (code)
                {
                    LogWarn("boost::asio::async_connect returned error code: {} {}", code.value(), code.message());
                }

                co_await m_ssl_socket.async_handshake(boost::asio::ssl::stream_base::client,
                                                      boost::asio::redirect_error(boost::asio::use_awaitable, code));
            }
            catch (const std::exception& e)
            {
                LogError("Exception thrown: {}", e.what());
                code = boost::asio::error::operation_aborted;
            }
        }

        /// @brief Writes the given request to the socket
        /// @param req The request to write
        void Write(const boost::beast::http::request<boost::beast::http::string_body>& req) override
        {
            try
            {
                boost::beast::http::write(m_ssl_socket, req);
            }
            catch (const std::exception& e)
            {
                LogError("Exception thrown during write: {}", e.what());
            }
        }

        /// @brief Asynchronous version of Write
        /// @param req The request to write
        /// @param ec The error code to store the result in
        boost::asio::awaitable<void> AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req,
                                                boost::beast::error_code& ec) override
        {
            try
            {
                co_await boost::beast::http::async_write(
                    m_ssl_socket, req, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
            }
            catch (const std::exception& e)
            {
                LogError("Exception thrown during async write: {}", e.what());
                ec = boost::asio::error::operation_aborted;
            }
        }

        /// @brief Reads a response from the socket
        /// @param res The response to read
        void Read(boost::beast::http::response<boost::beast::http::dynamic_body>& res) override
        {
            boost::beast::flat_buffer buffer;
            boost::beast::http::read(m_ssl_socket, buffer, res);
        }

        /// @brief Reads a response from the socket and writes it to a file
        /// @param res The response to read
        /// @param dstFilePath The path to the file to write to
        void ReadToFile(boost::beast::http::response_parser<boost::beast::http::dynamic_body>& res,
                        const std::string& dstFilePath) override
        {
            http_client_utils::ReadToFile(m_ssl_socket, res, dstFilePath);
        }

        /// @brief Asynchronous version of Read
        /// @param res The response to read
        /// @param ec The error code to store the result in
        boost::asio::awaitable<void> AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                               boost::beast::error_code& ec) override
        {
            boost::beast::flat_buffer buffer;
            co_await boost::beast::http::async_read(
                m_ssl_socket, buffer, res, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        }

        /// @brief Closes the socket
        void Close() override
        {
            try
            {
                m_ssl_socket.shutdown();
            }
            catch (const std::exception& e)
            {
                LogError("Exception thrown on socket closing: {}", e.what());
            }
        }

    private:
        /// @brief The SSL context to use for the socket
        boost::asio::ssl::context m_ctx;

        /// @brief The SSL socket to use for the connection
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_ssl_socket;
    };
} // namespace http_client
