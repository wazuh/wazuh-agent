#include <gtest/gtest.h>

#include <centralized_configuration.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

using centralized_configuration::CentralizedConfiguration;

namespace
{
    // NOLINTBEGIN(cppcoreguidelines-avoid-reference-coroutine-parameters)
    boost::asio::awaitable<void> TestExecuteCommand(CentralizedConfiguration& centralizedConfiguration,
                                                    const std::string& command,
                                                    const nlohmann::json& parameters,
                                                    module_command::Status expectedErrorCode)
    {
        const auto commandResult = co_await centralizedConfiguration.ExecuteCommand(command, parameters);
        EXPECT_EQ(commandResult.ErrorCode, expectedErrorCode);
    }

    // NOLINTEND(cppcoreguidelines-avoid-reference-coroutine-parameters)
} // namespace

TEST(CentralizedConfiguration, Constructor)
{
    EXPECT_NO_THROW([[maybe_unused]] CentralizedConfiguration centralizedConfiguration);
}

TEST(CentralizedConfiguration, ExecuteCommandReturnsFailureOnUnrecognizedCommand)
{
    boost::asio::io_context io_context;

    boost::asio::co_spawn(
        io_context,
        []() -> boost::asio::awaitable<void>
        {
            CentralizedConfiguration centralizedConfiguration;
            co_await TestExecuteCommand(
                centralizedConfiguration, "unknown-command", {}, module_command::Status::FAILURE);
        }(),
        boost::asio::detached);

    io_context.run();
}

TEST(CentralizedConfiguration, ExecuteCommandReturnsFailureOnEmptyList)
{
    boost::asio::io_context io_context;

    boost::asio::co_spawn(
        io_context,
        []() -> boost::asio::awaitable<void>
        {
            CentralizedConfiguration centralizedConfiguration;
            co_await TestExecuteCommand(centralizedConfiguration, "set-group", {}, module_command::Status::FAILURE);
        }(),
        boost::asio::detached);

    io_context.run();
}

TEST(CentralizedConfiguration, ExecuteCommandReturnsFailureOnParseParameters)
{
    boost::asio::io_context io_context;

    boost::asio::co_spawn(
        io_context,
        []() -> boost::asio::awaitable<void>
        {
            CentralizedConfiguration centralizedConfiguration;

            const std::vector<std::string> parameterList = {true, "group2"};
            co_await TestExecuteCommand(
                centralizedConfiguration, "set-group", parameterList, module_command::Status::FAILURE);
        }(),
        boost::asio::detached);

    io_context.run();
}

TEST(CentralizedConfiguration, ExecuteCommandHandlesRecognizedCommands)
{
    boost::asio::io_context io_context;

    boost::asio::co_spawn(
        io_context,
        []() -> boost::asio::awaitable<void>
        {
            CentralizedConfiguration centralizedConfiguration;
            centralizedConfiguration.SetGroupIdFunction([](const std::vector<std::string>&) {});
            centralizedConfiguration.GetGroupIdFunction([]() { return std::vector<std::string> {"group1", "group2"}; });
            centralizedConfiguration.SetDownloadGroupFilesFunction([](const std::string&, const std::string&)
                                                                   { return true; });

            const nlohmann::json groupsList = nlohmann::json::parse(R"([["group1", "group2"]])");

            co_await TestExecuteCommand(
                centralizedConfiguration, "set-group", groupsList, module_command::Status::SUCCESS);

            co_await TestExecuteCommand(centralizedConfiguration, "update-group", {}, module_command::Status::SUCCESS);

            co_await TestExecuteCommand(
                centralizedConfiguration, "unknown-command", {}, module_command::Status::FAILURE);
        }(),
        boost::asio::detached);

    io_context.run();
}

TEST(CentralizedConfiguration, SetFunctionsAreCalledAndReturnsCorrectResultsForSetGroup)
{
    boost::asio::io_context io_context;

    boost::asio::co_spawn(
        io_context,
        []() -> boost::asio::awaitable<void>
        {
            CentralizedConfiguration centralizedConfiguration;

            const nlohmann::json groupsList = nlohmann::json::parse(R"([["group1", "group2"]])");

            co_await TestExecuteCommand(
                centralizedConfiguration, "set-group", groupsList, module_command::Status::FAILURE);

            bool wasSetGroupIdFunctionCalled = false;
            bool wasDownloadGroupFilesFunctionCalled = false;

            centralizedConfiguration.SetGroupIdFunction([&wasSetGroupIdFunctionCalled](const std::vector<std::string>&)
                                                        { wasSetGroupIdFunctionCalled = true; });

            centralizedConfiguration.SetDownloadGroupFilesFunction(
                [&wasDownloadGroupFilesFunctionCalled](const std::string&, const std::string&)
                {
                    wasDownloadGroupFilesFunctionCalled = true;
                    return wasDownloadGroupFilesFunctionCalled;
                });

            EXPECT_FALSE(wasSetGroupIdFunctionCalled);
            EXPECT_FALSE(wasDownloadGroupFilesFunctionCalled);

            co_await TestExecuteCommand(
                centralizedConfiguration, "set-group", groupsList, module_command::Status::SUCCESS);

            EXPECT_TRUE(wasSetGroupIdFunctionCalled);
            EXPECT_TRUE(wasDownloadGroupFilesFunctionCalled);
        }(),
        boost::asio::detached);

    io_context.run();
}

TEST(CentralizedConfiguration, SetFunctionsAreCalledAndReturnsCorrectResultsForUpdateGroup)
{
    boost::asio::io_context io_context;

    boost::asio::co_spawn(
        io_context,
        []() -> boost::asio::awaitable<void>
        {
            CentralizedConfiguration centralizedConfiguration;

            co_await TestExecuteCommand(centralizedConfiguration, "update-group", {}, module_command::Status::FAILURE);

            bool wasGetGroupIdFunctionCalled = false;
            bool wasDownloadGroupFilesFunctionCalled = false;

            centralizedConfiguration.GetGroupIdFunction(
                [&wasGetGroupIdFunctionCalled]()
                {
                    wasGetGroupIdFunctionCalled = true;
                    return std::vector<std::string> {"group1", "group2"};
                });

            centralizedConfiguration.SetDownloadGroupFilesFunction(
                [&wasDownloadGroupFilesFunctionCalled](const std::string&, const std::string&)
                {
                    wasDownloadGroupFilesFunctionCalled = true;
                    return wasDownloadGroupFilesFunctionCalled;
                });

            EXPECT_FALSE(wasGetGroupIdFunctionCalled);
            EXPECT_FALSE(wasDownloadGroupFilesFunctionCalled);

            co_await TestExecuteCommand(centralizedConfiguration, "update-group", {}, module_command::Status::SUCCESS);

            EXPECT_TRUE(wasGetGroupIdFunctionCalled);
            EXPECT_TRUE(wasDownloadGroupFilesFunctionCalled);
        }(),
        boost::asio::detached);

    io_context.run();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
