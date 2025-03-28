#include <restart_handler.hpp>

#include <logger.hpp>

namespace restart_handler
{
    boost::asio::awaitable<module_command::CommandExecutionResult> RestartHandler::RestartAgent()
    {
        LogInfo("Restarting wazuh-agent");

        if (RunningAsService())
        {
            return RestartService();
        }
        else
        {
            return RestartForeground();
        }
    }
} // namespace restart_handler
