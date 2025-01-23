#include <gmock/gmock.h>
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

#include <filesystem>
#include <ifilesystem.hpp>

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

class MockFileSystem : public IFileSystem
{
public:
    MOCK_METHOD(bool, exists, (const std::filesystem::path& path), (const, override));
    MOCK_METHOD(bool, is_directory, (const std::filesystem::path& path), (const, override));
    MOCK_METHOD(std::uintmax_t, remove_all, (const std::filesystem::path& path), (override));
    MOCK_METHOD(std::filesystem::path, temp_directory_path, (), (const, override));
    MOCK_METHOD(bool, create_directories, (const std::filesystem::path& path), (override));
    MOCK_METHOD(void, rename, (const std::filesystem::path& from, const std::filesystem::path& to), (override));
    MOCK_METHOD(bool, remove, (const std::filesystem::path& path), (override));
};

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
            CentralizedConfiguration centralizedConfiguration;
            centralizedConfiguration.SetGroupIdFunction([](const std::vector<std::string>&) { return true; });
            centralizedConfiguration.SetDownloadGroupFilesFunction(
                [](std::string, std::string) -> boost::asio::awaitable<bool> { co_return true; });
            centralizedConfiguration.ValidateFileFunction([](const std::filesystem::path&) { return true; });
            centralizedConfiguration.ReloadModulesFunction([]() {});

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

            CentralizedConfiguration centralizedConfiguration(std::move(mockFileSystem));
            centralizedConfiguration.SetGroupIdFunction([](const std::vector<std::string>&) { return true; });
            centralizedConfiguration.GetGroupIdFunction([]() { return std::vector<std::string> {"group1", "group2"}; });
            centralizedConfiguration.SetDownloadGroupFilesFunction(
                [](std::string, std::string) -> boost::asio::awaitable<bool> { co_return true; });
            centralizedConfiguration.ValidateFileFunction([](const std::filesystem::path&) { return true; });
            centralizedConfiguration.ReloadModulesFunction([]() {});

            const nlohmann::json groupsList = nlohmann::json::parse(R"({"groups":["group1", "group2"]})");

            co_await TestExecuteCommand(centralizedConfiguration,
                                        "set-group",
                                        groupsList,
                                        module_command::Status::SUCCESS,
                                        "CentralizedConfiguration set-group done.");

            co_await TestExecuteCommand(centralizedConfiguration,
                                        "update-group",
                                        {},
                                        module_command::Status::SUCCESS,
                                        "CentralizedConfiguration update-group done.");

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

            CentralizedConfiguration centralizedConfiguration(std::move(mockFileSystem));

            const nlohmann::json groupsList = nlohmann::json::parse(R"({"groups":["group1", "group2"]})");

            bool wasSetGroupIdFunctionCalled = false;
            bool wasDownloadGroupFilesFunctionCalled = false;

            centralizedConfiguration.SetGroupIdFunction(
                [&wasSetGroupIdFunctionCalled](const std::vector<std::string>&)
                {
                    wasSetGroupIdFunctionCalled = true;
                    return true;
                });

            // NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
            centralizedConfiguration.SetDownloadGroupFilesFunction(
                [&wasDownloadGroupFilesFunctionCalled](std::string, std::string) -> boost::asio::awaitable<bool>
                {
                    wasDownloadGroupFilesFunctionCalled = true;
                    co_return wasDownloadGroupFilesFunctionCalled;
                });
            // NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)

            centralizedConfiguration.ValidateFileFunction([](const std::filesystem::path&) { return true; });
            centralizedConfiguration.ReloadModulesFunction([]() {});

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

            CentralizedConfiguration centralizedConfiguration(std::move(mockFileSystem));

            bool wasGetGroupIdFunctionCalled = false;
            bool wasDownloadGroupFilesFunctionCalled = false;

            centralizedConfiguration.GetGroupIdFunction(
                [&wasGetGroupIdFunctionCalled]()
                {
                    wasGetGroupIdFunctionCalled = true;
                    return std::vector<std::string> {"group1", "group2"};
                });

            // NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
            centralizedConfiguration.SetDownloadGroupFilesFunction(
                [&wasDownloadGroupFilesFunctionCalled](std::string, std::string) -> boost::asio::awaitable<bool>
                {
                    wasDownloadGroupFilesFunctionCalled = true;
                    co_return wasDownloadGroupFilesFunctionCalled;
                });
            // NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)

            centralizedConfiguration.ValidateFileFunction([](const std::filesystem::path&) { return true; });
            centralizedConfiguration.ReloadModulesFunction([]() {});

            EXPECT_FALSE(wasGetGroupIdFunctionCalled);
            EXPECT_FALSE(wasDownloadGroupFilesFunctionCalled);

            co_await TestExecuteCommand(centralizedConfiguration,
                                        "update-group",
                                        {},
                                        module_command::Status::SUCCESS,
                                        "CentralizedConfiguration update-group done.");

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
