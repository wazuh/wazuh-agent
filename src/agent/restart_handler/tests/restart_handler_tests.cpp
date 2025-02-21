#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <restart_handler_unix.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace testing;

namespace
{
    // NOLINTBEGIN(cppcoreguidelines-avoid-reference-signal-handler-definitions)
    void SigchldHandler(int status)
    {
        while (waitpid(-1, &status, WNOHANG) > 0)
        {
            // The child process has terminated, no action needed here
        }
    }

    // NOLINTEND(cppcoreguidelines-avoid-reference-signal-handler-definitions)

    void SigtermHandler([[maybe_unused]] int signum)
    {
        exit(0);
    }
} // namespace

TEST(TestUsingSystemctl, ReturnsTrueWhenSystemctlIsAvailable)
{
    setenv("INVOCATION_ID", "testing", 1);
    if (0 == std::system("which systemctl > /dev/null 2>&1"))
    {
        EXPECT_TRUE(restart_handler::RunningAsService());
    }
    else
    {
        EXPECT_FALSE(restart_handler::RunningAsService());
    }
}

TEST(TestUsingSystemctl, ReturnsFalseWhenSystemctlIsNotAvailable)
{
    unsetenv("INVOCATION_ID");
    EXPECT_FALSE(restart_handler::RunningAsService());
}

TEST(StopAgentTest, GracefullyStop)
{
    testing::internal::CaptureStdout();
    signal(SIGCHLD, SigchldHandler);

    pid_t pid = fork();

    if (pid < 0)
    {
        FAIL() << "Fork failed!";
    }
    if (pid == 0)
    {
        // Child process (Simulate agent running)
        const int timeout = 60;
        std::this_thread::sleep_for(std::chrono::seconds(timeout));
    }

    // Parent process
    std::this_thread::sleep_for(std::chrono::seconds(1));

    const int timeout = 10;
    restart_handler::StopAgent(pid, timeout);

    std::string stdout_logs = testing::internal::GetCapturedStdout();
    EXPECT_EQ(stdout_logs.find("Timeout reached! Forcing agent process termination."), std::string::npos);
    EXPECT_NE(stdout_logs.find("Agent stopped"), std::string::npos);
}

TEST(StopAgentTest, ForceStop)
{
    testing::internal::CaptureStdout();
    signal(SIGCHLD, SigchldHandler);
    signal(SIGTERM, SIG_IGN); // Ignore SIGTERM

    pid_t pid = fork();

    if (pid < 0)
    {
        FAIL() << "Fork failed!";
    }
    if (pid == 0)
    {
        // Child process (Simulate agent running)
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }

    // Parent process
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    restart_handler::StopAgent(pid, 2);

    std::string stdout_logs = testing::internal::GetCapturedStdout();
    EXPECT_NE(stdout_logs.find("Timeout reached! Forcing agent process termination."), std::string::npos);
    EXPECT_NE(stdout_logs.find("Agent stopped"), std::string::npos);
}

TEST(RestartWithForkTest, ShouldRestartUsingFork)
{

    testing::internal::CaptureStdout();
    pid_t pid = fork();

    if (pid < 0)
    {
        FAIL() << "Fork failed!";
    }
    if (pid == 0)
    {
        // Child process (Simulate agent running)
        setsid();
        signal(SIGTERM, SigtermHandler);

        boost::asio::io_context io_context;

        boost::asio::co_spawn(
            io_context,
            []() -> boost::asio::awaitable<void>
            {
                const char* args[] = {"/usr/bin/sleep", "3"};
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
                restart_handler::RestartHandler::SetCommandLineArguments(2, const_cast<char**>(args));

                const auto result = co_await restart_handler::RestartWithFork();
            }(),
            boost::asio::detached);

        io_context.run();
    }

    // Parent process
    int status {};
    waitpid(pid, &status, 0);

    std::string stdout_logs = testing::internal::GetCapturedStdout();
    EXPECT_EQ(stdout_logs.find("Agent stopped"), std::string::npos);
    EXPECT_EQ(stdout_logs.find("Starting wazuh agent in a new process"), std::string::npos);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
