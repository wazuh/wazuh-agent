#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

class ITestCommandsProcessingTaskFunctions
{
public:
    virtual ~ITestCommandsProcessingTaskFunctions() = default;

    virtual std::optional<module_command::CommandEntry> GetCommandFromQueue() = 0;
    virtual void PopCommandFromQueue() = 0;
    virtual void ReportCommandResult(module_command::CommandEntry& cmd) = 0;
    virtual boost::asio::awaitable<module_command::CommandExecutionResult>
    DispatchCommand(module_command::CommandEntry& cmd) = 0;
};

class MockTestCommandsProcessingTaskFunctions : public ITestCommandsProcessingTaskFunctions
{
public:
    MOCK_METHOD(std::optional<module_command::CommandEntry>, GetCommandFromQueue, (), (override));
    MOCK_METHOD(void, PopCommandFromQueue, (), (override));
    MOCK_METHOD(void, ReportCommandResult, (module_command::CommandEntry&), (override));
    MOCK_METHOD(boost::asio::awaitable<module_command::CommandExecutionResult>,
                DispatchCommand,
                (module_command::CommandEntry&),
                (override));
};
