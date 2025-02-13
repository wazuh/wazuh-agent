#pragma once

#include <command_entry.hpp>

#include <boost/asio/awaitable.hpp>

#include <vector>

namespace restart_handler
{
    /// @brief Class for handling service restarts.
    class RestartHandler
    {
    public:
        /// @brief A list of command line arguments used to restart the agent process.
        static std::vector<char*> startupCmdLineArgs;

        /// @brief Timeout for the restart operation until force a hard restart.
        static const int timeoutInSecs = 30;

        /// @brief RestartHandler class.
        explicit RestartHandler();

        /// @brief Stores the command-line arguments passed to the agent.
        /// @param argc Number of arguments.
        /// @param argv Array of arguments.
        static void SetCommandLineArguments(int argc, char* argv[])
        {
            for (int i = 0; i < argc; ++i)
            {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                RestartHandler::startupCmdLineArgs.emplace_back(argv[i]);
            }
            // Add a nullptr to terminate the argument list, as required by execve.
            RestartHandler::startupCmdLineArgs.emplace_back(nullptr);
        }

        /// @brief Executes the restart command.
        /// @return Result of the restart command execution.
        static boost::asio::awaitable<module_command::CommandExecutionResult> RestartAgent();
    };
} // namespace restart_handler
