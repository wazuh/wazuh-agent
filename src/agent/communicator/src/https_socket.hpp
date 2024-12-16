#include "http_client_utils.hpp"
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
        /// @param ec The error code, if any occurred
        /// @param timeOut The timeout for the connection
        void Connect(boost::asio::io_context& io_context,
                     const boost::asio::ip::tcp::resolver::results_type& endpoints,
                     boost::system::error_code& ec,
                     const std::chrono::seconds timeOut = std::chrono::seconds(TIMEOUT_DEFAULT)) override
        {
            try
            {
                bool connectionSuccess = false;

                // Start the asynchronous connect operation
                boost::asio::async_connect(m_ssl_socket.lowest_layer(),
                                           endpoints,
                                           [&](const boost::system::error_code& errorCode, const auto&)
                                           {
                                               if (!errorCode)
                                               {
                                                   connectionSuccess = true;
                                                   ec = errorCode;
                                                   LogDebug("Connected successfully");
                                               }
                                           });

                io_context.run_for(timeOut); // Run for 2 seconds();

                if (connectionSuccess)
                {
                    m_ssl_socket.handshake(boost::asio::ssl::stream_base::client, ec);
                    if (ec)
                    {
                        LogDebug("Handshake failed: {}", ec.message());
                        return;
                    }
                }
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown: {}", e.what());
                ec = boost::asio::error::operation_aborted;
            }
        }

        /// @brief Asynchronous version of Connect
        /// @param endpoints The endpoints to connect to
        /// @param ec The error code, if any occurred
        /// @param timeOut The timeout for the connection
        boost::asio::awaitable<void> AsyncConnect(
            const boost::asio::ip::tcp::resolver::results_type& endpoints,
            boost::system::error_code& ec,
            [[maybe_unused]] const std::chrono::seconds timeOut = std::chrono::seconds(TIMEOUT_DEFAULT)) override
        {
            using namespace boost::asio::experimental::awaitable_operators;

            auto timer = std::make_shared<boost::asio::steady_timer>(co_await boost::asio::this_coro::executor);
            timer->expires_after(timeOut);

            auto result = std::make_shared<boost::system::error_code>();
            auto taskCompleted = std::make_shared<bool>(false);

            try
            {
                co_await (http_client_utils::TimerTask(timer, result, taskCompleted) ||
                          http_client_utils::SocketConnectTask(
                              m_ssl_socket.lowest_layer(), endpoints, result, taskCompleted));

                if (!result)
                {
                    LogDebug("Connection error:  {}", result->value());
                }
                else
                {
                    co_await m_ssl_socket.async_handshake(boost::asio::ssl::stream_base::client,
                                                          boost::asio::redirect_error(boost::asio::use_awaitable, ec));
                    if (ec)
                    {
                        LogDebug("Handshake failed: {}", ec.message());
                    }
                }
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown: {}", e.what());
                ec = boost::asio::error::operation_aborted;
            }
        }

        /// @brief Writes the given request to the socket
        /// @param req The request to write
        /// @param ec The error code, if any occurred
        void Write(const boost::beast::http::request<boost::beast::http::string_body>& req,
                   boost::system::error_code& ec) override
        {
            try
            {
                boost::beast::http::write(m_ssl_socket, req, ec);
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown during write: {}", e.what());
            }
        }

        /// @brief Asynchronous version of Write
        /// @param req The request to write
        /// @param ec The error code, if any occurred
        boost::asio::awaitable<void> AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req,
                                                boost::system::error_code& ec) override
        {
            try
            {
                co_await boost::beast::http::async_write(
                    m_ssl_socket, req, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown during async write: {}", e.what());
                ec = boost::asio::error::operation_aborted;
            }
        }

        /// @brief Reads a response from the socket
        /// @param res The response to read
        /// @param ec The error code, if any occurred
        void Read(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                  boost::system::error_code& ec) override
        {
            try
            {
                boost::beast::flat_buffer buffer;
                boost::beast::http::read(m_ssl_socket, buffer, res, ec);
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown during read: {}", e.what());
            }
        }

        /// @brief Reads a response from the socket and writes it to a file
        /// @param res The response to read
        /// @param dstFilePath The path to the file to write to
        void ReadToFile(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                        const std::string& dstFilePath) override
        {
            try
            {
                http_client_utils::ReadToFile(m_ssl_socket, res, dstFilePath);
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown during read to file: {}", e.what());
            }
        }

        /// @brief Asynchronous version of Read
        /// @param res The response to read
        /// @param ec The error code, if any occurred
        boost::asio::awaitable<void> AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                               boost::system::error_code& ec) override
        {
            try
            {
                boost::beast::flat_buffer buffer;
                co_await boost::beast::http::async_read(
                    m_ssl_socket, buffer, res, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown during async read: {}", e.what());
                ec = boost::asio::error::operation_aborted;
            }
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
                LogDebug("Exception thrown on socket closing: {}", e.what());
            }
        }

    private:
        /// @brief The SSL context to use for the socket
        boost::asio::ssl::context m_ctx;

        /// @brief The SSL socket to use for the connection
        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_ssl_socket;
    };
} // namespace http_client
