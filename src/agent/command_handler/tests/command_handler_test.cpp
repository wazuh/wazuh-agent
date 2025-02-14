#include <gtest/gtest.h>

#include <command_handler.hpp>
#include <mock_command_processing_task_functions.hpp>
#include <mock_command_store.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

// NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines,cppcoreguidelines-avoid-reference-coroutine-parameters)

using namespace testing;

TEST(CommandHandlerConstructorTest, CommandHandlerConstructor)
{
    auto configurationParser = std::make_shared<configuration::ConfigurationParser>();

    auto mockCommandStore = std::make_unique<command_store::MockCommandStore>();

    EXPECT_NO_THROW(const command_handler::CommandHandler cm(configurationParser, std::move(mockCommandStore)));
}

TEST(CommandHandlerConstructorTest, CommandHandlerConstructorNoConfigParser)
{
    auto mockCommandStore = std::make_unique<command_store::MockCommandStore>();

    EXPECT_THROW(const command_handler::CommandHandler cm(nullptr, std::move(mockCommandStore)), std::runtime_error);
}

class CommandHandlerTest : public Test
{
protected:
    void SetUp() override
    {
        auto mockCommandStorePtr = std::make_unique<command_store::MockCommandStore>();
        m_mockCommandStore = mockCommandStorePtr.get();

        m_configurationParser = std::make_shared<configuration::ConfigurationParser>();
        m_commandHandler =
            std::make_unique<command_handler::CommandHandler>(m_configurationParser, std::move(mockCommandStorePtr));

        m_mockCommandFunctions = std::make_unique<MockTestCommandsProcessingTaskFunctions>();

        m_mockGetCommandFromQueue = [this]()
        {
            m_commandHandler->Stop();
            return m_mockCommandFunctions->GetCommandFromQueue();
        };

        m_mockPopCommandFromQueue = [this]()
        {
            m_mockCommandFunctions->PopCommandFromQueue();
        };

        m_mockReportCommandResult = [this](module_command::CommandEntry& cmd)
        {
            m_mockCommandFunctions->ReportCommandResult(cmd);
        };

        m_mockDispatchCommand =
            [this](module_command::CommandEntry& cmd) -> boost::asio::awaitable<module_command::CommandExecutionResult>
        {
            co_return co_await m_mockCommandFunctions->DispatchCommand(cmd);
        };
    }

    void TearDown() override {}

    command_store::MockCommandStore* m_mockCommandStore = nullptr;
    std::shared_ptr<configuration::ConfigurationParser> m_configurationParser;
    std::unique_ptr<command_handler::CommandHandler> m_commandHandler;
    std::unique_ptr<MockTestCommandsProcessingTaskFunctions> m_mockCommandFunctions;
    std::function<std::optional<module_command::CommandEntry>()> m_mockGetCommandFromQueue;
    std::function<void()> m_mockPopCommandFromQueue;
    std::function<void(module_command::CommandEntry&)> m_mockReportCommandResult;
    std::function<boost::asio::awaitable<module_command::CommandExecutionResult>(module_command::CommandEntry&)>
        m_mockDispatchCommand;
};

TEST_F(CommandHandlerTest, CommandsProcessingTaskProcessesCommandSetGroupSuccessfully)
{
    module_command::CommandEntry testCommand;
    nlohmann::json parameters;
    parameters[module_command::GROUPS_ARG] = nlohmann::json::array({"group1", "group2"});
    testCommand.Id = "command-id-1";
    testCommand.Command = module_command::SET_GROUP_COMMAND;
    testCommand.Parameters = parameters;

    EXPECT_CALL(*m_mockCommandStore, GetCommandByStatus(_)).WillOnce(Return(std::nullopt));

    EXPECT_CALL(*m_mockCommandFunctions, GetCommandFromQueue()).WillOnce(Return(testCommand));

    EXPECT_CALL(*m_mockCommandStore, StoreCommand(_)).WillOnce(Return(true));

    EXPECT_CALL(*m_mockCommandFunctions, PopCommandFromQueue()).Times(1);

    module_command::CommandExecutionResult result;
    result.ErrorCode = module_command::Status::SUCCESS;
    EXPECT_CALL(*m_mockCommandFunctions, DispatchCommand(_))
        .WillOnce([](module_command::CommandEntry&) -> boost::asio::awaitable<module_command::CommandExecutionResult>
                  { co_return module_command::CommandExecutionResult {module_command::Status::SUCCESS}; });

    EXPECT_CALL(*m_mockCommandStore, UpdateCommand(_)).WillOnce(Return(true));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        m_commandHandler->CommandsProcessingTask(
            m_mockGetCommandFromQueue, m_mockPopCommandFromQueue, m_mockReportCommandResult, m_mockDispatchCommand),
        boost::asio::detached);
    ioContext.run();
}

TEST_F(CommandHandlerTest, CommandsProcessingTaskProcessesCommandSetGroupSuccessfullyExtraParameter)
{
    module_command::CommandEntry testCommand;
    nlohmann::json parameters;
    parameters[module_command::GROUPS_ARG] = nlohmann::json::array({"group1", "group2"});
    parameters["extra-arg"] = "test-extra-arg";
    testCommand.Id = "command-id-1";
    testCommand.Command = module_command::SET_GROUP_COMMAND;
    testCommand.Parameters = parameters;

    EXPECT_CALL(*m_mockCommandStore, GetCommandByStatus(_)).WillOnce(Return(std::nullopt));

    EXPECT_CALL(*m_mockCommandFunctions, GetCommandFromQueue()).WillOnce(Return(testCommand));

    EXPECT_CALL(*m_mockCommandStore, StoreCommand(_)).WillOnce(Return(true));

    EXPECT_CALL(*m_mockCommandFunctions, PopCommandFromQueue()).Times(1);

    module_command::CommandExecutionResult result;
    result.ErrorCode = module_command::Status::SUCCESS;
    EXPECT_CALL(*m_mockCommandFunctions, DispatchCommand(_))
        .WillOnce([](module_command::CommandEntry&) -> boost::asio::awaitable<module_command::CommandExecutionResult>
                  { co_return module_command::CommandExecutionResult {module_command::Status::SUCCESS}; });

    EXPECT_CALL(*m_mockCommandStore, UpdateCommand(_)).WillOnce(Return(true));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        m_commandHandler->CommandsProcessingTask(
            m_mockGetCommandFromQueue, m_mockPopCommandFromQueue, m_mockReportCommandResult, m_mockDispatchCommand),
        boost::asio::detached);
    ioContext.run();
}

TEST_F(CommandHandlerTest, CommandsProcessingTaskProcessesCommandFetchConfigSuccessfully)
{
    module_command::CommandEntry testCommand;
    testCommand.Id = "command-id-1";
    testCommand.Command = module_command::FETCH_CONFIG_COMMAND;

    EXPECT_CALL(*m_mockCommandStore, GetCommandByStatus(_)).WillOnce(Return(std::nullopt));

    EXPECT_CALL(*m_mockCommandFunctions, GetCommandFromQueue()).WillOnce(Return(testCommand));

    EXPECT_CALL(*m_mockCommandStore, StoreCommand(_)).WillOnce(Return(true));

    EXPECT_CALL(*m_mockCommandFunctions, PopCommandFromQueue()).Times(1);

    module_command::CommandExecutionResult result;
    result.ErrorCode = module_command::Status::SUCCESS;
    EXPECT_CALL(*m_mockCommandFunctions, DispatchCommand(_))
        .WillOnce([](module_command::CommandEntry&) -> boost::asio::awaitable<module_command::CommandExecutionResult>
                  { co_return module_command::CommandExecutionResult {module_command::Status::SUCCESS}; });

    EXPECT_CALL(*m_mockCommandStore, UpdateCommand(_)).WillOnce(Return(true));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        m_commandHandler->CommandsProcessingTask(
            m_mockGetCommandFromQueue, m_mockPopCommandFromQueue, m_mockReportCommandResult, m_mockDispatchCommand),
        boost::asio::detached);
    ioContext.run();
}

TEST_F(CommandHandlerTest, CommandsProcessingTaskProcessesCommandFetchConfigWithParameterSuccessfully)
{
    module_command::CommandEntry testCommand;
    nlohmann::json parameters;
    parameters[module_command::GROUPS_ARG] = "string-no-array";
    testCommand.Id = "command-id-1";
    testCommand.Command = module_command::FETCH_CONFIG_COMMAND;
    testCommand.Parameters = parameters;

    EXPECT_CALL(*m_mockCommandStore, GetCommandByStatus(_)).WillOnce(Return(std::nullopt));

    EXPECT_CALL(*m_mockCommandFunctions, GetCommandFromQueue()).WillOnce(Return(testCommand));

    EXPECT_CALL(*m_mockCommandStore, StoreCommand(_)).WillOnce(Return(true));

    EXPECT_CALL(*m_mockCommandFunctions, PopCommandFromQueue()).Times(1);

    module_command::CommandExecutionResult result;
    result.ErrorCode = module_command::Status::SUCCESS;
    EXPECT_CALL(*m_mockCommandFunctions, DispatchCommand(_))
        .WillOnce([](module_command::CommandEntry&) -> boost::asio::awaitable<module_command::CommandExecutionResult>
                  { co_return module_command::CommandExecutionResult {module_command::Status::SUCCESS}; });

    EXPECT_CALL(*m_mockCommandStore, UpdateCommand(_)).WillOnce(Return(true));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        m_commandHandler->CommandsProcessingTask(
            m_mockGetCommandFromQueue, m_mockPopCommandFromQueue, m_mockReportCommandResult, m_mockDispatchCommand),
        boost::asio::detached);
    ioContext.run();
}

TEST_F(CommandHandlerTest, CommandsProcessingTaskProcessesCommandRestartSuccessfully)
{
    module_command::CommandEntry testCommand;
    testCommand.Id = "command-id-1";
    testCommand.Command = module_command::RESTART_COMMAND;

    EXPECT_CALL(*m_mockCommandStore, GetCommandByStatus(_)).WillOnce(Return(std::nullopt));

    EXPECT_CALL(*m_mockCommandFunctions, GetCommandFromQueue()).WillOnce(Return(testCommand));

    EXPECT_CALL(*m_mockCommandStore, StoreCommand(_)).WillOnce(Return(true));

    EXPECT_CALL(*m_mockCommandFunctions, PopCommandFromQueue()).Times(1);

    module_command::CommandExecutionResult result;
    result.ErrorCode = module_command::Status::SUCCESS;
    EXPECT_CALL(*m_mockCommandFunctions, DispatchCommand(_))
        .WillOnce([](module_command::CommandEntry&) -> boost::asio::awaitable<module_command::CommandExecutionResult>
                  { co_return module_command::CommandExecutionResult {module_command::Status::SUCCESS}; });

    EXPECT_CALL(*m_mockCommandStore, UpdateCommand(_)).WillOnce(Return(true));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        m_commandHandler->CommandsProcessingTask(
            m_mockGetCommandFromQueue, m_mockPopCommandFromQueue, m_mockReportCommandResult, m_mockDispatchCommand),
        boost::asio::detached);
    ioContext.run();
}

TEST_F(CommandHandlerTest, CommandsProcessingTaskProcessesCommandRestartWithParameterSuccessfully)
{
    module_command::CommandEntry testCommand;
    nlohmann::json parameters;
    parameters[module_command::GROUPS_ARG] = "string-no-array";
    testCommand.Id = "command-id-1";
    testCommand.Command = module_command::RESTART_COMMAND;
    testCommand.Parameters = parameters;

    EXPECT_CALL(*m_mockCommandStore, GetCommandByStatus(_)).WillOnce(Return(std::nullopt));

    EXPECT_CALL(*m_mockCommandFunctions, GetCommandFromQueue()).WillOnce(Return(testCommand));

    EXPECT_CALL(*m_mockCommandStore, StoreCommand(_)).WillOnce(Return(true));

    EXPECT_CALL(*m_mockCommandFunctions, PopCommandFromQueue()).Times(1);

    module_command::CommandExecutionResult result;
    result.ErrorCode = module_command::Status::SUCCESS;
    EXPECT_CALL(*m_mockCommandFunctions, DispatchCommand(_))
        .WillOnce([](module_command::CommandEntry&) -> boost::asio::awaitable<module_command::CommandExecutionResult>
                  { co_return module_command::CommandExecutionResult {module_command::Status::SUCCESS}; });

    EXPECT_CALL(*m_mockCommandStore, UpdateCommand(_)).WillOnce(Return(true));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        m_commandHandler->CommandsProcessingTask(
            m_mockGetCommandFromQueue, m_mockPopCommandFromQueue, m_mockReportCommandResult, m_mockDispatchCommand),
        boost::asio::detached);
    ioContext.run();
}

TEST_F(CommandHandlerTest, CommandsProcessingTaskNotCommand)
{
    EXPECT_CALL(*m_mockCommandStore, GetCommandByStatus(_)).WillOnce(Return(std::nullopt));

    EXPECT_CALL(*m_mockCommandFunctions, GetCommandFromQueue()).WillOnce(Return(std::nullopt));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        m_commandHandler->CommandsProcessingTask(
            m_mockGetCommandFromQueue, m_mockPopCommandFromQueue, m_mockReportCommandResult, m_mockDispatchCommand),
        boost::asio::detached);
    ioContext.run();
}

TEST_F(CommandHandlerTest, CommandsProcessingTaskCheckCommandInvalidCommand)
{
    module_command::CommandEntry testCommand;
    nlohmann::json parameters;
    parameters[module_command::GROUPS_ARG] = nlohmann::json::array({"group1", "group2"});
    testCommand.Id = "command-id-1";
    testCommand.Command = "invalid";
    testCommand.Parameters = parameters;

    EXPECT_CALL(*m_mockCommandStore, GetCommandByStatus(_)).WillOnce(Return(std::nullopt));

    EXPECT_CALL(*m_mockCommandFunctions, GetCommandFromQueue()).WillOnce(Return(testCommand));
    EXPECT_CALL(*m_mockCommandFunctions, ReportCommandResult(_)).Times(1);
    EXPECT_CALL(*m_mockCommandFunctions, PopCommandFromQueue()).Times(1);

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        m_commandHandler->CommandsProcessingTask(
            m_mockGetCommandFromQueue, m_mockPopCommandFromQueue, m_mockReportCommandResult, m_mockDispatchCommand),
        boost::asio::detached);
    ioContext.run();
}

TEST_F(CommandHandlerTest, CommandsProcessingTaskCheckCommandSetGroupEmptyParameters)
{
    module_command::CommandEntry testCommand;
    nlohmann::json parameters;
    testCommand.Id = "command-id-1";
    testCommand.Command = module_command::SET_GROUP_COMMAND;
    testCommand.Parameters = parameters;

    EXPECT_CALL(*m_mockCommandStore, GetCommandByStatus(_)).WillOnce(Return(std::nullopt));

    EXPECT_CALL(*m_mockCommandFunctions, GetCommandFromQueue()).WillOnce(Return(testCommand));
    EXPECT_CALL(*m_mockCommandFunctions, ReportCommandResult(_)).Times(1);
    EXPECT_CALL(*m_mockCommandFunctions, PopCommandFromQueue()).Times(1);

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        m_commandHandler->CommandsProcessingTask(
            m_mockGetCommandFromQueue, m_mockPopCommandFromQueue, m_mockReportCommandResult, m_mockDispatchCommand),
        boost::asio::detached);
    ioContext.run();
}

TEST_F(CommandHandlerTest, CommandsProcessingTaskCheckCommandSetGroupArgNoArray)
{
    module_command::CommandEntry testCommand;
    nlohmann::json parameters;
    parameters[module_command::GROUPS_ARG] = "string-no-array";
    testCommand.Id = "command-id-1";
    testCommand.Command = module_command::SET_GROUP_COMMAND;
    testCommand.Parameters = parameters;

    EXPECT_CALL(*m_mockCommandStore, GetCommandByStatus(_)).WillOnce(Return(std::nullopt));

    EXPECT_CALL(*m_mockCommandFunctions, GetCommandFromQueue()).WillOnce(Return(testCommand));
    EXPECT_CALL(*m_mockCommandFunctions, ReportCommandResult(_)).Times(1);
    EXPECT_CALL(*m_mockCommandFunctions, PopCommandFromQueue()).Times(1);

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        m_commandHandler->CommandsProcessingTask(
            m_mockGetCommandFromQueue, m_mockPopCommandFromQueue, m_mockReportCommandResult, m_mockDispatchCommand),
        boost::asio::detached);
    ioContext.run();
}

TEST_F(CommandHandlerTest, CommandsProcessingTaskStoreCommandFail)
{
    module_command::CommandEntry testCommand;
    nlohmann::json parameters;
    parameters[module_command::GROUPS_ARG] = nlohmann::json::array({"group1", "group2"});
    testCommand.Id = "command-id-1";
    testCommand.Command = module_command::SET_GROUP_COMMAND;
    testCommand.Parameters = parameters;

    EXPECT_CALL(*m_mockCommandStore, GetCommandByStatus(_)).WillOnce(Return(std::nullopt));

    EXPECT_CALL(*m_mockCommandFunctions, GetCommandFromQueue()).WillOnce(Return(testCommand));

    EXPECT_CALL(*m_mockCommandStore, StoreCommand(_)).WillOnce(Return(false));

    EXPECT_CALL(*m_mockCommandFunctions, ReportCommandResult(_)).Times(1);
    EXPECT_CALL(*m_mockCommandFunctions, PopCommandFromQueue()).Times(1);

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        m_commandHandler->CommandsProcessingTask(
            m_mockGetCommandFromQueue, m_mockPopCommandFromQueue, m_mockReportCommandResult, m_mockDispatchCommand),
        boost::asio::detached);
    ioContext.run();
}

TEST_F(CommandHandlerTest, CommandsProcessingTaskCleanUpInProgressCommands)
{
    module_command::CommandExecutionResult result;
    result.ErrorCode = module_command::Status::IN_PROGRESS;

    module_command::CommandEntry testCommand;
    testCommand.Id = "command-id-1";
    testCommand.Command = module_command::SET_GROUP_COMMAND;
    testCommand.ExecutionResult = result;

    module_command::CommandEntry testCommand2;
    testCommand2.Id = "command-id-2";
    testCommand2.Command = module_command::RESTART_COMMAND;
    testCommand2.ExecutionResult = result;

    std::vector<module_command::CommandEntry> commands = {testCommand, testCommand2};
    std::optional<std::vector<module_command::CommandEntry>> optionalCommands = commands;

    EXPECT_CALL(*m_mockCommandStore, GetCommandByStatus(_)).WillOnce(Return(optionalCommands));

    EXPECT_CALL(*m_mockCommandFunctions, ReportCommandResult(_)).Times(2);
    EXPECT_CALL(*m_mockCommandStore, UpdateCommand(_)).Times(2);

    EXPECT_CALL(*m_mockCommandFunctions, GetCommandFromQueue()).WillOnce(Return(std::nullopt));

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        m_commandHandler->CommandsProcessingTask(
            m_mockGetCommandFromQueue, m_mockPopCommandFromQueue, m_mockReportCommandResult, m_mockDispatchCommand),
        boost::asio::detached);
    ioContext.run();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines,cppcoreguidelines-avoid-reference-coroutine-parameters)
