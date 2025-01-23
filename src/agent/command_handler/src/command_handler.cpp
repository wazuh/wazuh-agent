#include <command_handler.hpp>

namespace command_handler
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

    void CommandHandler::Stop()
    {
        m_keepRunning.store(false);
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
} // namespace command_handler
