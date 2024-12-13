#include "http_client_utils.hpp"
#include <ihttp_socket.hpp>
#include <logger.hpp>

#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
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
        /// @param io_context The io_context associated to the socket
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

                boost::asio::async_connect(m_socket,
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
                if (!connectionSuccess)
                {
                    ec = boost::asio::error::timed_out;
                    LogDebug("Connection timed out");
                }
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown: {}", e.what());
            }
        }

        /// @brief Starts a timer and updates the error code and task completion status accordingly
        /// @param timer The timer to start
        /// @param result The error code to update
        /// @param taskCompleted In/Out Indicates whether the socket task is completed and, if not, serves as a flag
        /// that indicates TimeOut

        boost::asio::awaitable<void> TimerTask(std::shared_ptr<boost::asio::steady_timer> timer,
                                               std::shared_ptr<boost::system::error_code> result,
                                               std::shared_ptr<bool> taskCompleted)
        {
            try
            {
                co_await timer->async_wait(boost::asio::use_awaitable);

                if (!(*taskCompleted))
                {
                    LogDebug("Connection timed out");
                    *result = boost::asio::error::timed_out;
                    *taskCompleted = true;
                }
            }
            catch (const boost::system::system_error& e)
            {
                if (!(*taskCompleted) && e.code() != boost::asio::error::operation_aborted)
                {
                    *result = boost::asio::error::fault;
                }
            }
        }

        /// @brief Performs the socket connection
        /// @param endpoints The endpoints to connect to
        /// @param result The error code, if any occurred
        /// @param taskCompleted In/Out Indicates whether the timer task is completed and, if not, serves as a flag that
        /// indicates the socket is connected
        boost::asio::awaitable<void> SocketTask(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                std::shared_ptr<boost::system::error_code> result,
                                                std::shared_ptr<bool> taskCompleted)
        {
            boost::system::error_code socketErrorCode;
            co_await boost::asio::async_connect(
                m_socket, endpoints, boost::asio::redirect_error(boost::asio::use_awaitable, socketErrorCode));
            if (!(*taskCompleted) && !socketErrorCode)
            {
                LogDebug("Connected successfully");
                result->clear();
                *taskCompleted = true;
            }
            else
            {
                *result = socketErrorCode;
            }
        }

        /// @brief Asynchronous version of Connect
        /// @param endpoints The endpoints to connect to
        /// @param ec The error code, if any occurred
        /// @param timeOut The timeout for the connection
        boost::asio::awaitable<void>
        AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                     boost::system::error_code& ec,
                     const std::chrono::seconds timeOut = std::chrono::seconds(TIMEOUT_DEFAULT)) override
        {
            using namespace boost::asio::experimental::awaitable_operators;

            auto timer = std::make_shared<boost::asio::steady_timer>(co_await boost::asio::this_coro::executor);
            timer->expires_after(timeOut);

            auto result = std::make_shared<boost::system::error_code>();
            auto taskCompleted = std::make_shared<bool>(false);

            try
            {
                co_await (TimerTask(timer, result, taskCompleted) || SocketTask(endpoints, result, taskCompleted));
                if (!result)
                {
                    LogDebug("Connection error:  {}", result->value());
                }
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown during async connect: {}", e.what());
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
                boost::beast::http::write(m_socket, req, ec);
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
                    m_socket, req, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
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
                boost::beast::http::read(m_socket, buffer, res, ec);
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
                http_client_utils::ReadToFile(m_socket, res, dstFilePath);
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
                    m_socket, buffer, res, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
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
                m_socket.close();
            }
            catch (const std::exception& e)
            {
                LogDebug("Exception thrown on socket closing: {}", e.what());
            }
        }

    private:
        /// @brief The socket to use for the HTTP connection
        boost::asio::ip::tcp::socket m_socket;
    };
} // namespace http_client
