#include <command_handler.hpp>

namespace command_handler
{
    const std::unordered_map<std::string, std::string> VALID_COMMANDS_MAP = {
        {"set-group", "CentralizedConfiguration"}, {"update-group", "CentralizedConfiguration"},
        {"restart", "restart"}};

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

            cmd.Module = it->second;
            return true;
        }
        else
        {
            LogError("The command {} is not valid.", cmd.Command);
            return false;
        }
    }
} // namespace command_handler
