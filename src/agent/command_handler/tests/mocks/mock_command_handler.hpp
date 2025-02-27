#pragma once

#include <gmock/gmock.h>

#include <icommand_handler.hpp>

#include <boost/asio/awaitable.hpp>

namespace command_handler
{
    class MockCommandHandler : public command_handler::ICommandHandler
    {
    public:
        MOCK_METHOD(boost::asio::awaitable<void>,
                    CommandsProcessingTask,
                    (const std::function<std::optional<module_command::CommandEntry>()>,
                     const std::function<void()>,
                     const std::function<void(module_command::CommandEntry&)>,
                     const std::function<boost::asio::awaitable<module_command::CommandExecutionResult>(
                         module_command::CommandEntry&)>),
                    (override));

        MOCK_METHOD(void, Stop, (), (override));
    };
} // namespace command_handler
