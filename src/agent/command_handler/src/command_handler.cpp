#include <command_handler.hpp>
#include <command_store.hpp>

namespace
{
    struct CommandDetails
    {
        std::string module;
        module_command::CommandExecutionMode executionMode;
        std::vector<std::pair<std::string, nlohmann::json::value_t>> arguments;
    };

    const std::unordered_map<std::string, CommandDetails> VALID_COMMANDS_MAP = {
        {"set-group",
         CommandDetails {"CentralizedConfiguration",
                         module_command::CommandExecutionMode::SYNC,
                         {{"groups", nlohmann::json::value_t::array}}}},
        {"update-group", CommandDetails {"CentralizedConfiguration", module_command::CommandExecutionMode::SYNC, {}}}};
} // namespace

namespace command_handler
{
    CommandHandler::CommandHandler(std::shared_ptr<configuration::ConfigurationParser> configurationParser)
    {
        if (!configurationParser)
        {
            throw std::runtime_error(std::string("Invalid Configuration Parser passed."));
        }

        auto dbFolderPath =
            configurationParser->GetConfig<std::string>("agent", "path.data").value_or(config::DEFAULT_DATA_PATH);

        try
        {
            m_commandStore = std::make_unique<command_store::CommandStore>(dbFolderPath);
        }
        catch (const std::exception& e)
        {
            LogError("Error creating persistence: {}.", e.what());
        }
    }

    CommandHandler::~CommandHandler() = default;

    boost::asio::awaitable<void> CommandHandler::CommandsProcessingTask(
        const std::function<std::optional<module_command::CommandEntry>()>
            getCommandFromQueue,                         // NOLINT(performance-unnecessary-value-param)
        const std::function<void()> popCommandFromQueue, // NOLINT(performance-unnecessary-value-param)
        const std::function<void(module_command::CommandEntry&)>
            reportCommandResult, // NOLINT(performance-unnecessary-value-param)
        const std::function<boost::asio::awaitable<module_command::CommandExecutionResult>(
            module_command::CommandEntry&)> dispatchCommand) // NOLINT(performance-unnecessary-value-param)
    {
        using namespace std::chrono_literals;
        const auto executor = co_await boost::asio::this_coro::executor;
        std::unique_ptr<boost::asio::steady_timer> expTimer = std::make_unique<boost::asio::steady_timer>(executor);

        CleanUpInProgressCommands(reportCommandResult);

        while (m_keepRunning.load())
        {
            auto cmd = getCommandFromQueue();
            if (cmd == std::nullopt)
            {
                expTimer->expires_after(1000ms);
                co_await expTimer->async_wait(boost::asio::use_awaitable);
                continue;
            }

            LogDebug("Processing command: {}({})", cmd.value().Command, cmd.value().Parameters.dump());

            if (!CheckCommand(cmd.value()))
            {
                cmd.value().ExecutionResult.ErrorCode = module_command::Status::FAILURE;
                cmd.value().ExecutionResult.Message = "Command is not valid";
                LogError("Error checking module and args for command: {} {}. Error: {}",
                         cmd.value().Id,
                         cmd.value().Command,
                         cmd.value().ExecutionResult.Message);
                reportCommandResult(cmd.value());
                popCommandFromQueue();
                continue;
            }

            if (!m_commandStore->StoreCommand(cmd.value()))
            {
                cmd.value().ExecutionResult.ErrorCode = module_command::Status::FAILURE;
                cmd.value().ExecutionResult.Message = "Agent's database failure";
                LogError("Error storing command: {} {}. Error: {}",
                         cmd.value().Id,
                         cmd.value().Command,
                         cmd.value().ExecutionResult.Message);
                reportCommandResult(cmd.value());
                popCommandFromQueue();
                continue;
            }

            popCommandFromQueue();

            if (cmd.value().ExecutionMode == module_command::CommandExecutionMode::SYNC)
            {
                cmd.value().ExecutionResult = co_await dispatchCommand(cmd.value());
                m_commandStore->UpdateCommand(cmd.value());
                LogInfo("Done processing command: {}({})", cmd.value().Command, cmd.value().Module);
            }
            else
            {
                // NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
                co_spawn(
                    executor,
                    [cmd, dispatchCommand, this]() mutable -> boost::asio::awaitable<void>
                    {
                        cmd.value().ExecutionResult = co_await dispatchCommand(cmd.value());
                        m_commandStore->UpdateCommand(cmd.value());
                        LogInfo("Done processing command: {}({})", cmd.value().Command, cmd.value().Module);
                        co_return;
                    },
                    boost::asio::detached);
                // NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)
            }
        }
    }

    void
    CommandHandler::CleanUpInProgressCommands(std::function<void(module_command::CommandEntry&)>
                                                  reportCommandResult) // NOLINT(performance-unnecessary-value-param)
    {
        auto cmds = m_commandStore->GetCommandByStatus(module_command::Status::IN_PROGRESS);

        if (cmds.has_value())
        {
            for (auto& cmd : cmds.value())
            {
                cmd.ExecutionResult.ErrorCode = module_command::Status::FAILURE;
                cmd.ExecutionResult.Message = "Agent stopped during execution";
                reportCommandResult(cmd);
                m_commandStore->UpdateCommand(cmd);
            }
        }
    }

    bool CommandHandler::CheckCommand(module_command::CommandEntry& cmd)
    {
        auto itMap = VALID_COMMANDS_MAP.find(cmd.Command);
        if (itMap != VALID_COMMANDS_MAP.end())
        {
            const auto& details = itMap->second;
            for (const auto& arg : details.arguments)
            {
                if (!cmd.Parameters.contains(arg.first) || cmd.Parameters[arg.first].type() != arg.second)
                {
                    LogError("The command {} parameters are invalid or missing: {}.", cmd.Command, arg.first);
                    return false;
                }
            }

            for (auto it = cmd.Parameters.begin(); it != cmd.Parameters.end(); ++it)
            {
                const std::string& key = it.key();
                auto itValidaArgs = std::find_if(details.arguments.begin(),
                                                 details.arguments.end(),
                                                 [&key](const auto& pair) { return pair.first == key; });
                if (itValidaArgs == details.arguments.end())
                {
                    LogWarn("The command {} has extra parameter: {}. It will be ignored.", cmd.Command, key);
                }
            }
            cmd.Module = details.module;
            cmd.ExecutionMode = details.executionMode;
            return true;
        }
        else
        {
            LogError("The command {} is not valid.", cmd.Command);
            return false;
        }
    }

    void CommandHandler::Stop()
    {
        m_keepRunning.store(false);
    }
} // namespace command_handler
