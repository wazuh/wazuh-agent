#include <command_handler_utils.hpp>

#include <logger.hpp>

std::tuple<command_store::Status, std::string> DispatchCommand(const command_store::CommandEntry& commandEntry,
                                                               std::shared_ptr<ModuleWrapper> module,
                                                               std::shared_ptr<IMultiTypeQueue> messageQueue)
{
    if (!module)
    {
        LogError("Error dispatching command: module {} not found", commandEntry.Module);
        return std::make_tuple(command_store::Status::FAILURE, "Module not found");
    }

    LogInfo("Dispatching command {}({})", commandEntry.Command, commandEntry.Module);

    const auto result = module->Command(commandEntry.Command);

    Message message {MessageType::STATEFUL, {{"data", result}, {"module", module->Name()}}};
    messageQueue->push(message);

    return std::make_tuple(command_store::Status::SUCCESS, result);
}
