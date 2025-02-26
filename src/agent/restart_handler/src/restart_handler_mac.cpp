#include <restart_handler.hpp>

#include <logger.hpp>

namespace restart_handler
{
    bool RunningAsService()
    {
        return (getppid() == 1);
    }

    boost::asio::awaitable<module_command::CommandExecutionResult> RestartService()
    {
        if (std::system("launchctl kickstart -k system/com.wazuh.agent") != 0)
        {
            co_return module_command::CommandExecutionResult {module_command::Status::IN_PROGRESS,
                                                              "launchctl restart execution"};
        }
        else
        {
            LogError("launchctl failed to restart Agent");
            co_return module_command::CommandExecutionResult {module_command::Status::FAILURE,
                                                              "launchctl restart failed"};
        }
    }
} // namespace restart_handler
