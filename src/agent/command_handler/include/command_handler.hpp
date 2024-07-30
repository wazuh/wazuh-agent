#pragma once
#include <iostream>

#include <boost/asio.hpp>
#include <optional>
#include <queue>

namespace command_handler
{
    class CommandHandler
    {
    public:
        template<typename T>
        boost::asio::awaitable<void> ProcessCommandsFromQueue(const std::function<std::optional<T>()> GetCommand)
        {
            using namespace std::chrono_literals;
            const auto executor = co_await boost::asio::this_coro::executor;
            std::unique_ptr<boost::asio::steady_timer> expTimer = std::make_unique<boost::asio::steady_timer>(executor);

            while (m_keepRunning.load())
            {
                auto cmd = GetCommand();
                if (cmd == std::nullopt)
                {
                    expTimer->expires_after(1000ms);
                    co_await expTimer->async_wait(boost::asio::use_awaitable);
                    continue;
                }
            }
        }

    private:
        std::atomic<bool> m_keepRunning = true;
    };
} // namespace command_handler
