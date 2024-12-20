#include "http_client_utils.hpp"

#include <logger.hpp>

namespace http_client_utils
{
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
                *result = e.code();
            }
        }
    }
} // namespace http_client_utils
