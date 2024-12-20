#pragma once
#include <ihttp_socket.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/system.hpp>
#include <logger.hpp>

#include <memory>
#include <string>

namespace http_client_utils
{
    /// @brief Timeout for HTTP requests
    constexpr int TIMEOUT_SECONDS = 60;

    /// @brief Starts a timer and updates the error code and task completion status accordingly
    /// @param timer The timer to start
    /// @param result The error code to update
    /// @param taskCompleted In/Out Indicates whether the socket task is completed and, if not, serves as a flag
    /// that indicates TimeOut
    boost::asio::awaitable<void> TimerTask(std::shared_ptr<boost::asio::steady_timer> timer,
                                           std::shared_ptr<boost::system::error_code> result,
                                           std::shared_ptr<bool> taskCompleted);

    /// @brief Performs the socket connection
    /// @param socket The socket to connect
    /// @param endpoints The endpoints to connect to
    /// @param result The error code, if any occurred
    /// @param taskCompleted In/Out Indicates whether the timer task is completed and, if not, serves as a flag that
    /// indicates the socket is connected
    template<typename SocketType>
    boost::asio::awaitable<void> SocketConnectTask(SocketType& socket,
                                                   const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                   std::shared_ptr<boost::system::error_code> result,
                                                   std::shared_ptr<bool> taskCompleted)
    {
        boost::system::error_code socketErrorCode;

        co_await boost::asio::async_connect(
            socket, endpoints, boost::asio::redirect_error(boost::asio::use_awaitable, socketErrorCode));

        if (!(*taskCompleted) && !socketErrorCode)
        {
            result->clear();
            *taskCompleted = true;
        }
        else
        {
            *result = socketErrorCode;
        }
    }

    /// @brief Performs the socket write
    /// @param socket The socket to write to
    /// @param req The request to write
    /// @param result The error code, if any occurred
    /// @param taskCompleted In/Out Indicates whether the timer task is completed and, if not, serves as a flag that
    /// indicates the socket is connected
    template<typename SocketType>
    boost::asio::awaitable<void>
    SocketWriteTask(SocketType& socket,
                    const boost::beast::http::request<boost::beast::http::string_body>& req,
                    std::shared_ptr<boost::system::error_code> result,
                    std::shared_ptr<bool> taskCompleted)
    {
        boost::system::error_code socketErrorCode;

        co_await boost::beast::http::async_write(
            socket, req, boost::asio::redirect_error(boost::asio::use_awaitable, socketErrorCode));

        if (!(*taskCompleted) && !socketErrorCode)
        {
            result->clear();
            *taskCompleted = true;
        }
        else
        {
            *result = socketErrorCode;
        }
    }

    /// @brief Performs the socket read
    /// @param socket The socket to read from
    /// @param buffer The buffer to read into
    /// @param res The response to read
    /// @param result The error code, if any occurred
    /// @param taskCompleted In/Out Indicates whether the timer task is completed and, if not, serves as a flag that
    /// indicates the socket is connected
    template<typename SocketType>
    boost::asio::awaitable<void> SocketReadTask(SocketType& socket,
                                                boost::beast::flat_buffer& buffer,
                                                boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                                std::shared_ptr<boost::system::error_code> result,
                                                std::shared_ptr<bool> taskCompleted)
    {
        boost::system::error_code socketErrorCode;

        co_await boost::beast::http::async_read(
            socket, buffer, res, boost::asio::redirect_error(boost::asio::use_awaitable, socketErrorCode));

        if (!(*taskCompleted) && !socketErrorCode)
        {
            result->clear();
            *taskCompleted = true;
        }
        else
        {
            *result = socketErrorCode;
        }
    }
} // namespace http_client_utils
