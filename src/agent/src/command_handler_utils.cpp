#include <command_handler_utils.hpp>
#include <imultitype_queue.hpp>

#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/system_error.hpp>
#include <logger.hpp>

#include <chrono>

namespace
{
    using ExecuteFunction =
        std::function<boost::asio::awaitable<module_command::CommandExecutionResult>(std::string, nlohmann::json)>;

    boost::asio::awaitable<void> ExecuteCommandTask(ExecuteFunction executeFunction,
                                                    module_command::CommandEntry commandEntry,
                                                    std::shared_ptr<module_command::CommandExecutionResult> result)
    {
        try
        {
            *result = co_await executeFunction(commandEntry.Command, commandEntry.Parameters);
        }
        catch (const std::exception& e)
        {
            if (result->ErrorCode == module_command::Status::UNKNOWN)
            {
                result->ErrorCode = module_command::Status::FAILURE;
                result->Message = "Error during command execution: " + std::string(e.what());
            }
        }
    }

    boost::asio::awaitable<void> TimerTask(std::shared_ptr<module_command::CommandExecutionResult> result)
    {
        try
        {
            constexpr auto timeout = std::chrono::minutes {10};
            auto timer = std::make_shared<boost::asio::steady_timer>(co_await boost::asio::this_coro::executor);
            timer->expires_after(timeout);
            co_await timer->async_wait(boost::asio::use_awaitable);

            if (result->ErrorCode == module_command::Status::UNKNOWN)
            {
                result->ErrorCode = module_command::Status::TIMEOUT;
                result->Message = "Command timed out";
            }
        }
        catch (const boost::system::system_error& e)
        {
            if (e.code() != boost::asio::error::operation_aborted)
            {
                if (result->ErrorCode == module_command::Status::UNKNOWN)
                {
                    result->ErrorCode = module_command::Status::FAILURE;
                    result->Message = "System error: " + std::string(e.what());
                }
            }
        }
        catch (const std::exception& e)
        {
            if (result->ErrorCode == module_command::Status::UNKNOWN)
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

    auto result = std::make_shared<module_command::CommandExecutionResult>();

    co_await (TimerTask(result) || ExecuteCommandTask(executeFunction, commandEntry, result));

    commandEntry.ExecutionResult.ErrorCode = result->ErrorCode;
    commandEntry.ExecutionResult.Message = result->Message;

    ReportCommandResult(commandEntry, messageQueue);

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

void ReportCommandResult(const module_command::CommandEntry& commandEntry,
                         std::shared_ptr<IMultiTypeQueue> messageQueue)
{
    auto metadata = nlohmann::json::object();
    metadata["module"] = "command";
    metadata["id"] = commandEntry.Id;
    metadata["operation"] = "update";

    nlohmann::json resultJson;
    resultJson["command"]["result"]["code"] = commandEntry.ExecutionResult.ErrorCode;
    resultJson["command"]["result"]["message"] = commandEntry.ExecutionResult.Message;

    const Message message {MessageType::STATEFUL, {resultJson}, metadata["module"], "", metadata.dump()};
    messageQueue->push(message);
}
