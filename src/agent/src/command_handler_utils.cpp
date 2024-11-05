#include <command_handler_utils.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/system_error.hpp>
#include <logger.hpp>

#include <chrono>

namespace
{
    template<typename ExecuteFunction>
    boost::asio::awaitable<void> ExecuteCommandTask(ExecuteFunction executeFunction,
                                                    module_command::CommandEntry commandEntry,
                                                    std::shared_ptr<module_command::CommandExecutionResult> result,
                                                    std::shared_ptr<bool> commandCompleted,
                                                    std::shared_ptr<boost::asio::steady_timer> timer)
    {
        try
        {
            *result = co_await executeFunction(commandEntry.Command, commandEntry.Parameters);
            *commandCompleted = true;
            timer->cancel();
        }
        catch (const std::exception& e)
        {
            result->ErrorCode = module_command::Status::FAILURE;
            result->Message = "Error during command execution: " + std::string(e.what());
        }
    }

    boost::asio::awaitable<void> TimerTask(std::shared_ptr<boost::asio::steady_timer> timer,
                                           std::shared_ptr<module_command::CommandExecutionResult> result,
                                           std::shared_ptr<bool> commandCompleted)
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
        catch (const boost::system::system_error& e)
        {
            if (!(*commandCompleted) && e.code() != boost::asio::error::operation_aborted)
            {
                result->ErrorCode = module_command::Status::FAILURE;
                result->Message = "System error: " + std::string(e.what());
            }
        }
        catch (const std::exception& e)
        {
            if (!(*commandCompleted))
            {
                result->ErrorCode = module_command::Status::FAILURE;
                result->Message = "Unexpected error: " + std::string(e.what());
            }
        }
    }
} // namespace

boost::asio::awaitable<module_command::CommandExecutionResult>
DispatchCommand(module_command::CommandEntry commandEntry,
                std::function<boost::asio::awaitable<module_command::CommandExecutionResult>(
                    std::string command, nlohmann::json parameters)> executeFunction,
                std::shared_ptr<IMultiTypeQueue> messageQueue)
{
    using namespace boost::asio::experimental::awaitable_operators;

    LogInfo("Dispatching command {}({})", commandEntry.Command, commandEntry.Module);

    const auto timeout = std::chrono::minutes(60);
    auto timer = std::make_shared<boost::asio::steady_timer>(co_await boost::asio::this_coro::executor);
    timer->expires_after(timeout);

    auto result = std::make_shared<module_command::CommandExecutionResult>();
    auto commandCompleted = std::make_shared<bool>(false);

    co_await (TimerTask(timer, result, commandCompleted) ||
              ExecuteCommandTask(executeFunction, commandEntry, result, commandCompleted, timer));

    nlohmann::json resultJson;
    resultJson["error"] = result->ErrorCode;
    resultJson["message"] = result->Message;
    resultJson["id"] = commandEntry.Id;

    Message message {MessageType::STATEFUL, {resultJson}, "CommandHandler"};
    messageQueue->push(message);

    co_return *result;
}

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

    auto moduleExecuteFunction =
        [module](const std::string& command,
                 const nlohmann::json& parameters) -> boost::asio::awaitable<module_command::CommandExecutionResult>
    {
        return module->ExecuteCommand(command, parameters);
    };

    co_return co_await DispatchCommand(commandEntry, moduleExecuteFunction, messageQueue);
}
