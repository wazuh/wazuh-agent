#include <restart_handler_unix.hpp>

#include <logger.hpp>

namespace restart_handler
{
    bool UsingSystemctl()
    {
        return (0 == std::system("which systemctl > /dev/null 2>&1") && nullptr != std::getenv("INVOCATION_ID"));
    }

    boost::asio::awaitable<module_command::CommandExecutionResult> RestartWithSystemd()
    {
        if (std::system("systemctl restart wazuh-agent") != 0)
        {
            co_return module_command::CommandExecutionResult {module_command::Status::IN_PROGRESS,
                                                              "Systemctl restart execution"};
        }
        else
        {
            LogError("Systemctl restart failed");
            co_return module_command::CommandExecutionResult {module_command::Status::FAILURE,
                                                              "Systemctl restart failed"};
        }
    }
} // namespace restart_handler
