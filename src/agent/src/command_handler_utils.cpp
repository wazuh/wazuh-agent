#include <command_handler_utils.hpp>

#include <logger.hpp>

boost::asio::awaitable<std::tuple<command_store::Status, std::string>>
DispatchCommand(command_store::CommandEntry commandEntry,
                std::shared_ptr<ModuleWrapper> module,
                std::shared_ptr<IMultiTypeQueue> messageQueue)
{
    if (!module)
    {
        LogError("Error dispatching command: module {} not found", commandEntry.Module);
        co_return std::make_tuple(command_store::Status::FAILURE, "Module not found");
    }

    LogInfo("Dispatching command {}({})", commandEntry.Command, commandEntry.Module);

    const auto result = co_await module->Command(commandEntry.Command);

    nlohmann::json resultJson;
    resultJson["result"] = result;
    resultJson["id"] = commandEntry.Id;

    Message message {MessageType::STATEFUL, {resultJson}, "CommandHandler"};
    messageQueue->push(message);

    co_return std::make_tuple(command_store::Status::SUCCESS, result);
}
