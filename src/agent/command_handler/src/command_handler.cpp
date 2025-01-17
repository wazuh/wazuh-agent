#include <command_handler.hpp>

namespace command_handler
{
    const std::unordered_map<std::string, std::pair<std::string, module_command::CommandExecutionMode>>
        VALID_COMMANDS_MAP = {
            {"set-group", {"CentralizedConfiguration", module_command::CommandExecutionMode::SYNC}},
            {"update-group", {"CentralizedConfiguration", module_command::CommandExecutionMode::SYNC}}};

    void CommandHandler::Stop()
    {
        m_keepRunning.store(false);
    }

    bool CommandHandler::CheckCommand(module_command::CommandEntry& cmd)
    {
        auto it = VALID_COMMANDS_MAP.find(cmd.Command);
        if (it != VALID_COMMANDS_MAP.end())
        {
            if (!cmd.Parameters.empty())
            {
                for (const auto& param : cmd.Parameters.items())
                {
                    if (!param.value().is_string() || param.value().get<std::string>().empty())
                    {
                        LogError("The command {} parameters must be non-empty strings.", cmd.Command);
                        return false;
                    }
                }
            }

            cmd.Module = it->second.first;
            cmd.ExecutionMode = it->second.second;
            return true;
        }
        else
        {
            LogError("The command {} is not valid.", cmd.Command);
            return false;
        }
    }
} // namespace command_handler
