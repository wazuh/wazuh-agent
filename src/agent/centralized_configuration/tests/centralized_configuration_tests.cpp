#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <centralized_configuration.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

#include "../../../common/file_helper/filesystem/tests/mocks/mock_filesystem.hpp"
#include <filesystem>

using centralized_configuration::CentralizedConfiguration;
using namespace testing;

namespace
{
    // NOLINTBEGIN(cppcoreguidelines-avoid-reference-coroutine-parameters)
    boost::asio::awaitable<void> TestExecuteCommand(CentralizedConfiguration& centralizedConfiguration,
                                                    const std::string& command,
                                                    const nlohmann::json& parameters,
                                                    module_command::Status expectedErrorCode,
                                                    const std::string& expectedMessage)
    {
        const auto commandResult = co_await centralizedConfiguration.ExecuteCommand(command, parameters);
        EXPECT_EQ(commandResult.ErrorCode, expectedErrorCode);
        EXPECT_EQ(commandResult.Message, expectedMessage);
    }

    // NOLINTEND(cppcoreguidelines-avoid-reference-coroutine-parameters)
} // namespace

TEST(CentralizedConfiguration, Constructor)
{
    EXPECT_NO_THROW(CentralizedConfiguration centralizedConfiguration(
        [](const std::vector<std::string>&) { return true; },
        []() { return std::vector<std::string> {}; },
        [](std::string, std::string) -> boost::asio::awaitable<bool> { co_return true; },
        [](const std::filesystem::path&) { return true; },
        []() {},
        nullptr));
}

TEST(CentralizedConfiguration, ConstructorNoGetGroups)
{
    EXPECT_THROW(CentralizedConfiguration centralizedConfiguration(
                     nullptr,
                     []() { return std::vector<std::string> {}; },
                     [](std::string, std::string) -> boost::asio::awaitable<bool> { co_return true; },
                     [](const std::filesystem::path&) { return true; },
                     []() {},
                     nullptr),
                 std::runtime_error);
}

TEST(CentralizedConfiguration, ConstructorNoSetGroups)
{
    EXPECT_THROW(CentralizedConfiguration centralizedConfiguration(
                     [](const std::vector<std::string>&) { return true; },
                     nullptr,
                     [](std::string, std::string) -> boost::asio::awaitable<bool> { co_return true; },
                     [](const std::filesystem::path&) { return true; },
                     []() {},
                     nullptr),
                 std::runtime_error);
}

TEST(CentralizedConfiguration, ConstructorNoDownloadGroups)
{
    EXPECT_THROW(CentralizedConfiguration centralizedConfiguration([](const std::vector<std::string>&) { return true; },
                                                                   []() { return std::vector<std::string> {}; },
                                                                   nullptr,
                                                                   [](const std::filesystem::path&) { return true; },
                                                                   []() {},
                                                                   nullptr),
                 std::runtime_error);
}

TEST(CentralizedConfiguration, ConstructorNoValidateGroups)
{
    EXPECT_THROW(CentralizedConfiguration centralizedConfiguration(
                     [](const std::vector<std::string>&) { return true; },
                     []() { return std::vector<std::string> {}; },
                     [](std::string, std::string) -> boost::asio::awaitable<bool> { co_return true; },
                     nullptr,
                     []() {},
                     nullptr),
                 std::runtime_error);
}

TEST(CentralizedConfiguration, ConstructorNoReloadModules)
{
    EXPECT_THROW(CentralizedConfiguration centralizedConfiguration(
                     [](const std::vector<std::string>&) { return true; },
                     []() { return std::vector<std::string> {}; },
                     [](std::string, std::string) -> boost::asio::awaitable<bool> { co_return true; },
                     [](const std::filesystem::path&) { return true; },
                     nullptr,
                     nullptr),
                 std::runtime_error);
}

TEST(CentralizedConfiguration, ExecuteCommandReturnsFailureOnUnrecognizedCommand)
{
    boost::asio::io_context io_context;

    boost::asio::co_spawn(
        io_context,
        []() -> boost::asio::awaitable<void>
        {
            CentralizedConfiguration centralizedConfiguration(
                [](const std::vector<std::string>&) { return true; },
                []() { return std::vector<std::string> {}; },
                [](std::string, std::string) -> boost::asio::awaitable<bool> { co_return true; },
                [](const std::filesystem::path&) { return true; },
                []() {},
                nullptr);
            co_await TestExecuteCommand(centralizedConfiguration,
                                        "unknown-command",
                                        {},
                                        module_command::Status::FAILURE,
                                        "CentralizedConfiguration command not recognized");
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
            CentralizedConfiguration centralizedConfiguration(
                [](const std::vector<std::string>&) { return true; },
                []() { return std::vector<std::string> {}; },
                [](std::string, std::string) -> boost::asio::awaitable<bool> { co_return true; },
                [](const std::filesystem::path&) { return true; },
                []() {});

            const nlohmann::json parameterListCase1 = nlohmann::json::parse(R"({"groups":[true, "group2"]})");
            co_await TestExecuteCommand(centralizedConfiguration,
                                        "set-group",
                                        parameterListCase1,
                                        module_command::Status::FAILURE,
                                        "CentralizedConfiguration error while parsing parameters");

            const nlohmann::json parameterListCase2 = nlohmann::json::parse(R"({"wrongKey":["group1", "group2"]})");
            co_await TestExecuteCommand(centralizedConfiguration,
                                        "set-group",
                                        parameterListCase2,
                                        module_command::Status::FAILURE,
                                        "CentralizedConfiguration error while parsing parameters");

            const nlohmann::json parameterListCase3 = nlohmann::json::parse(R"({"groups":["", "group2"]})");
            co_await TestExecuteCommand(
                centralizedConfiguration,
                "set-group",
                parameterListCase3,
                module_command::Status::FAILURE,
                "CentralizedConfiguration group set failed, a group name can not be an empty string.");
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
            auto mockFileSystem = std::make_shared<MockFileSystem>();

            EXPECT_CALL(*mockFileSystem, exists(_)).WillRepeatedly(Return(false));
            EXPECT_CALL(*mockFileSystem, is_directory(_)).WillRepeatedly(Return(true));
            EXPECT_CALL(*mockFileSystem, remove_all(_)).WillRepeatedly(Return(0));
            EXPECT_CALL(*mockFileSystem, temp_directory_path())
                .WillRepeatedly(Return(std::filesystem::temp_directory_path()));
            EXPECT_CALL(*mockFileSystem, create_directories(_)).WillRepeatedly(Return(true));
            EXPECT_CALL(*mockFileSystem, remove(_)).WillRepeatedly(Return(true));
            EXPECT_CALL(*mockFileSystem, rename(_, _)).WillRepeatedly(Return());

            CentralizedConfiguration centralizedConfiguration(
                [](const std::vector<std::string>&) { return true; },
                []() { return std::vector<std::string> {"group1", "group2"}; },
                [](std::string, std::string) -> boost::asio::awaitable<bool> { co_return true; },
                [](const std::filesystem::path&) { return true; },
                []() {},
                std::move(mockFileSystem));

            const nlohmann::json groupsList = nlohmann::json::parse(R"({"groups":["group1", "group2"]})");

            co_await TestExecuteCommand(centralizedConfiguration,
                                        "set-group",
                                        groupsList,
                                        module_command::Status::SUCCESS,
                                        "CentralizedConfiguration set-group done.");

            co_await TestExecuteCommand(centralizedConfiguration,
                                        "fetch-config",
                                        {},
                                        module_command::Status::SUCCESS,
                                        "CentralizedConfiguration fetch-config done.");

            co_await TestExecuteCommand(centralizedConfiguration,
                                        "unknown-command",
                                        {},
                                        module_command::Status::FAILURE,
                                        "CentralizedConfiguration command not recognized");
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
            auto mockFileSystem = std::make_shared<MockFileSystem>();

            EXPECT_CALL(*mockFileSystem, exists(_)).WillRepeatedly(Return(false));
            EXPECT_CALL(*mockFileSystem, is_directory(_)).WillRepeatedly(Return(true));
            EXPECT_CALL(*mockFileSystem, remove_all(_)).WillRepeatedly(Return(0));
            EXPECT_CALL(*mockFileSystem, temp_directory_path())
                .WillRepeatedly(Return(std::filesystem::temp_directory_path()));
            EXPECT_CALL(*mockFileSystem, create_directories(_)).WillRepeatedly(Return(true));
            EXPECT_CALL(*mockFileSystem, remove(_)).WillRepeatedly(Return(true));
            EXPECT_CALL(*mockFileSystem, rename(_, _)).WillRepeatedly(Return());

            bool wasSetGroupIdFunctionCalled = false;
            bool wasDownloadGroupFilesFunctionCalled = false;

            CentralizedConfiguration centralizedConfiguration(
                // NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
                [&wasSetGroupIdFunctionCalled](const std::vector<std::string>&)
                {
                    wasSetGroupIdFunctionCalled = true;
                    return true;
                },
                []() { return std::vector<std::string> {}; },
                [&wasDownloadGroupFilesFunctionCalled](std::string, std::string) -> boost::asio::awaitable<bool>
                {
                    wasDownloadGroupFilesFunctionCalled = true;
                    co_return wasDownloadGroupFilesFunctionCalled;
                },
                // NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)
                [](const std::filesystem::path&) { return true; },
                []() {},
                std::move(mockFileSystem));

            const nlohmann::json groupsList = nlohmann::json::parse(R"({"groups":["group1", "group2"]})");

            EXPECT_FALSE(wasSetGroupIdFunctionCalled);
            EXPECT_FALSE(wasDownloadGroupFilesFunctionCalled);

            co_await TestExecuteCommand(centralizedConfiguration,
                                        "set-group",
                                        groupsList,
                                        module_command::Status::SUCCESS,
                                        "CentralizedConfiguration set-group done.");

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
            auto mockFileSystem = std::make_shared<MockFileSystem>();

            EXPECT_CALL(*mockFileSystem, exists(_)).WillRepeatedly(Return(false));
            EXPECT_CALL(*mockFileSystem, is_directory(_)).WillRepeatedly(Return(true));
            EXPECT_CALL(*mockFileSystem, remove_all(_)).WillRepeatedly(Return(0));
            EXPECT_CALL(*mockFileSystem, temp_directory_path())
                .WillRepeatedly(Return(std::filesystem::temp_directory_path()));
            EXPECT_CALL(*mockFileSystem, create_directories(_)).WillRepeatedly(Return(true));
            EXPECT_CALL(*mockFileSystem, remove(_)).WillRepeatedly(Return(true));
            EXPECT_CALL(*mockFileSystem, rename(_, _)).WillRepeatedly(Return());

            bool wasGetGroupIdFunctionCalled = false;
            bool wasDownloadGroupFilesFunctionCalled = false;

            CentralizedConfiguration centralizedConfiguration(
                [](const std::vector<std::string>&) { return true; },
                // NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
                [&wasGetGroupIdFunctionCalled]()
                {
                    wasGetGroupIdFunctionCalled = true;
                    return std::vector<std::string> {"group1", "group2"};
                },
                [&wasDownloadGroupFilesFunctionCalled](std::string, std::string) -> boost::asio::awaitable<bool>
                {
                    wasDownloadGroupFilesFunctionCalled = true;
                    co_return wasDownloadGroupFilesFunctionCalled;
                },
                // NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)
                [](const std::filesystem::path&) { return true; },
                []() {},
                std::move(mockFileSystem));

            EXPECT_FALSE(wasGetGroupIdFunctionCalled);
            EXPECT_FALSE(wasDownloadGroupFilesFunctionCalled);

            co_await TestExecuteCommand(centralizedConfiguration,
                                        "fetch-config",
                                        {},
                                        module_command::Status::SUCCESS,
                                        "CentralizedConfiguration fetch-config done.");

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
