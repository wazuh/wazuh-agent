#pragma once

#include <boost/asio.hpp>
#include <command_store.hpp>

#include <atomic>
#include <iostream>
#include <optional>
#include <queue>

namespace command_handler
{
    class CommandHandler
    {
    public:
        template<typename T>
        boost::asio::awaitable<void> ProcessCommandsFromQueue(
            const std::function<std::optional<T>()> GetCommandFromQueue,
            const std::function<void()> PopCommandFromQueue,
            const std::function<std::tuple<command_store::Status, std::string>(T&)> DispatchCommand)
        {
            using namespace std::chrono_literals;
            const auto executor = co_await boost::asio::this_coro::executor;
            std::unique_ptr<boost::asio::steady_timer> expTimer = std::make_unique<boost::asio::steady_timer>(executor);

            while (m_keepRunning.load())
            {
                auto cmd = GetCommandFromQueue();
                if (cmd == std::nullopt)
                {
                    expTimer->expires_after(1000ms);
                    co_await expTimer->async_wait(boost::asio::use_awaitable);
                    continue;
                }

                m_commandStore.StoreCommand(cmd.value());
                PopCommandFromQueue();
                auto result = DispatchCommand(cmd.value());

                cmd.value().m_status = std::get<0>(result);
                cmd.value().m_result = std::get<1>(result);
                m_commandStore.UpdateCommand(cmd.value());

                std::cout << "Done processing command" << "\n";
            }
        }

        void Stop();

    private:
        std::atomic<bool> m_keepRunning = true;
        command_store::CommandStore m_commandStore;
    };
} // namespace command_handler
