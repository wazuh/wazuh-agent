#include <gtest/gtest.h>

#include <centralized_configuration.hpp>
#include <configuration_parser.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_awaitable.hpp>

using centralized_configuration::CentralizedConfiguration;

namespace
{
    // NOLINTBEGIN(cppcoreguidelines-avoid-reference-coroutine-parameters)
    boost::asio::awaitable<void> TestExecuteCommand(
        CentralizedConfiguration& centralizedConfiguration,
        const std::string& command,
        module_command::Status expectedErrorCode)
    {
        const auto commandResult = co_await centralizedConfiguration.ExecuteCommand(command);
        EXPECT_EQ(commandResult.ErrorCode, expectedErrorCode);
    }

    boost::asio::awaitable<void> TestSetGroupIdFunction(
        CentralizedConfiguration& centralizedConfiguration,
        bool& wasSetGroupIdFunctionCalled)
    {
        EXPECT_FALSE(wasSetGroupIdFunctionCalled);

        co_await TestExecuteCommand(
            centralizedConfiguration,
            R"({"command": "set-group"})",
            module_command::Status::SUCCESS
        );

        EXPECT_TRUE(wasSetGroupIdFunctionCalled);
    }
    // NOLINTEND(cppcoreguidelines-avoid-reference-coroutine-parameters)
}

TEST(CentralizedConfiguration, Constructor)
{
    EXPECT_NO_THROW(
        [[maybe_unused]] CentralizedConfiguration centralizedConfiguration
    );
}

TEST(CentralizedConfiguration, ImplementsModuleWrapperInterface)
{
    CentralizedConfiguration centralizedConfiguration;
    EXPECT_NO_THROW(centralizedConfiguration.Start());
    EXPECT_NO_THROW(centralizedConfiguration.Stop());
    EXPECT_NO_THROW(centralizedConfiguration.Name());

    const std::string emptyConfig;
    configuration::ConfigurationParser configurationParser(emptyConfig);
    EXPECT_NO_THROW(centralizedConfiguration.Setup(configurationParser));
}

TEST(CentralizedConfiguration, ExecuteCommandReturnsFailureOnUnrecognizedCommand)
{
    CentralizedConfiguration centralizedConfiguration;

    boost::asio::io_context io_context;
    boost::asio::co_spawn(
        io_context,
        TestExecuteCommand(centralizedConfiguration, R"({"command": "unknown-command"})", module_command::Status::FAILURE),
        boost::asio::detached
    );

    io_context.run();
}

TEST(CentralizedConfiguration, ExecuteCommandHandlesRecognizedCommands)
{
    CentralizedConfiguration centralizedConfiguration;
    centralizedConfiguration.SetGroupIdFunction(
        [](const std::vector<std::string>&)
        {
        }
    );

    boost::asio::io_context io_context;

    boost::asio::co_spawn(
        io_context,
        TestExecuteCommand(centralizedConfiguration, R"({"command": "set-group"})", module_command::Status::SUCCESS),
        boost::asio::detached
    );

    boost::asio::co_spawn(
        io_context,
        TestExecuteCommand(centralizedConfiguration, R"({"command": "update-group"})", module_command::Status::SUCCESS),
        boost::asio::detached
    );

    io_context.run();
}

TEST(CentralizedConfiguration, SetGroupIdFunctionIsCalledAndReturnsCorrectResult)
{
    CentralizedConfiguration centralizedConfiguration;
    bool wasSetGroupIdFunctionCalled = false;

    centralizedConfiguration.SetGroupIdFunction(
        [&wasSetGroupIdFunctionCalled](const std::vector<std::string>&)
        {
            wasSetGroupIdFunctionCalled = true;
        }
    );

    boost::asio::io_context io_context;

    boost::asio::co_spawn(
        io_context,
        TestSetGroupIdFunction(centralizedConfiguration, wasSetGroupIdFunctionCalled),
        boost::asio::detached
    );

    io_context.run();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
