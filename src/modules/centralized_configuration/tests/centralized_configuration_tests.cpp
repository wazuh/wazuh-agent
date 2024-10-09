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
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-reference-coroutine-parameters)
    boost::asio::awaitable<void> TestExecuteCommandUnrecognized(CentralizedConfiguration& centralizedConfiguration)
    {
        const std::string command = R"({"command": "unknown-command"})";
        const auto commandResult = co_await centralizedConfiguration.ExecuteCommand(command);

        EXPECT_EQ(commandResult.ErrorCode, module_command::Status::FAILURE);
    }
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
        TestExecuteCommandUnrecognized(centralizedConfiguration),
        boost::asio::detached
    );

    io_context.run();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
