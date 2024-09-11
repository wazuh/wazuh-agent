#pragma once

#include <boost/asio.hpp>
#include <command_store.hpp>

#include <iostream>
#include <optional>
#include <queue>

namespace command_handler
{
    class CommandHandler
    {
    public:
        template<typename T>
        boost::asio::awaitable<void>
        ProcessCommandsFromQueue(const std::function<std::optional<T>()> GetCommandFromQueue,
                                 const std::function<bool()> PopCommandFromQueue,
                                 const std::function<int(T&)> DispatchCommand)
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
                DispatchCommand(cmd.value());

                cmd.value().m_status = command_store::Status::SUCCESS;
                cmd.value().m_result = "Successfully executed";
                m_commandStore.UpdateCommand(cmd.value());

                std::cout << "Done processing command" << "\n";
            }
        }

    private:
        std::atomic<bool> m_keepRunning = true;
        command_store::CommandStore m_commandStore;
    };
} // namespace command_handler
