#include <logger.hpp>
#include <restart_handler.hpp>

namespace restart_handler
{

    std::vector<char*> RestartHandler::startupCmdLineArgs;

    boost::asio::awaitable<module_command::CommandExecutionResult> RestartHandler::RestartCommand()
    {
        // TODO
        co_return module_command::CommandExecutionResult {module_command::Status::FAILURE,
                                                          "RestartHandler is not implemented yet"};
    }

} // namespace restart_handler
