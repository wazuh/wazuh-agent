#include <command_handler_utils.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/steady_timer.hpp>
#include <logger.hpp>

#include <chrono>

boost::asio::awaitable<module_command::CommandExecutionResult>
DispatchCommand(module_command::CommandEntry commandEntry,
                std::shared_ptr<ModuleWrapper> module,
                std::shared_ptr<IMultiTypeQueue> messageQueue)
{
    if (!module)
    {
        LogError("Error dispatching command: module {} not found", commandEntry.Module);
        co_return module_command::CommandExecutionResult {module_command::Status::FAILURE, "Module not found"};
    }

    LogInfo("Dispatching command {}({})", commandEntry.Command, commandEntry.Module);

    const auto timeout = std::chrono::minutes(60);
    auto timer = std::make_shared<boost::asio::steady_timer>(co_await boost::asio::this_coro::executor);
    timer->expires_after(timeout);

    auto result = std::make_shared<module_command::CommandExecutionResult>();
    auto commandCompleted = std::make_shared<bool>(false);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    auto executeCommandTask = [module, commandEntry, result, commandCompleted, timer]() -> boost::asio::awaitable<void>
    {
        *result = co_await module->ExecuteCommand(commandEntry.Command);
        *commandCompleted = true;
        timer->cancel();
    };

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    auto timerTask = [timer, result, commandCompleted]() -> boost::asio::awaitable<void>
    {
        try
        {
            co_await timer->async_wait(boost::asio::use_awaitable);

            if (!(*commandCompleted))
            {
                result->ErrorCode = module_command::Status::TIMEOUT;
                result->Message = "Command timed out";
            }
        }
        catch (const std::exception& e)
        {
            result->ErrorCode = module_command::Status::FAILURE;
            result->Message = "Error occurred while waiting for command to complete";
        }
    };

    co_await timerTask();
    co_await executeCommandTask();

    nlohmann::json resultJson;
    resultJson["error"] = result->ErrorCode;
    resultJson["message"] = result->Message;
    resultJson["id"] = commandEntry.Id;

    Message message {MessageType::STATEFUL, {resultJson}, "CommandHandler"};
    messageQueue->push(message);

    co_return *result;
}
