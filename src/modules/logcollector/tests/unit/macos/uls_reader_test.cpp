#include <gtest/gtest.h>

#include <uls_reader.hpp>

#include <boost/asio.hpp>

#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>

TEST(ULSReaderTest, Constructor)
{
    auto& logCollector = logcollector::Logcollector::Instance();
    logCollector.Stop();
    logcollector::ULSReader ulsReader(logCollector);
}

TEST(ULSReaderTest, RunAndStop)
{
    auto& logCollector = logcollector::Logcollector::Instance();
    logCollector.Stop();
    logCollector.SetPushMessageFunction(
        [](Message) -> int // NOLINT(performance-unnecessary-value-param)
        {
            return 0;
        }
    );

    boost::asio::io_context ioContext;

    auto didULSReaderRun = false;

    logcollector::ULSReader ulsReader(logCollector);
    boost::asio::co_spawn(
        ioContext,
        [&ulsReader, &didULSReaderRun]() -> boost::asio::awaitable<void> // NOLINT(cppcoreguidelines-avoid-capturing-lambda-coroutines)
        {
            co_await ulsReader.Run();
            didULSReaderRun = true;
        },
        boost::asio::detached
    );

    boost::asio::post(
        ioContext,
        [&ulsReader]() -> void
        {
            std::this_thread::sleep_for(std::chrono::seconds(1)); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            ulsReader.Stop();
        }
    );

    std::thread ioThread([&ioContext]() -> void
    {
        ioContext.run();
    });

    ioContext.run();

    EXPECT_TRUE(didULSReaderRun);

    ioThread.join();
}

TEST(ULSReaderTest, CreateQueryFromRule)
{
    auto& logCollector = logcollector::Logcollector::Instance();
    logCollector.Stop();
    logCollector.SetPushMessageFunction(
        [](Message) -> int // NOLINT(performance-unnecessary-value-param)
        {
            return 0;
        }
    );

    logcollector::ULSReader ulsReader(
        logCollector,
        50, // NOLINT
        "debug",
        R"(process != "sshd" OR message CONTAINS "invalid")",
        {"trace", "activity", "log"}
    );

    logcollector::ULSReader ulsReader2(
        logCollector,
        50, // NOLINT
        "debug",
        R"(process != "wazuh-agent"")",
        {"trace", "activity", "log"}
    );

    boost::asio::io_context ioContext;

    boost::asio::co_spawn(
        ioContext,
        [&ulsReader]() -> boost::asio::awaitable<void> // NOLINT(cppcoreguidelines-avoid-capturing-lambda-coroutines)
        {
            co_await ulsReader.Run();
        },
        boost::asio::detached
    );

    boost::asio::co_spawn(
        ioContext,
        [&ulsReader2]() -> boost::asio::awaitable<void> // NOLINT(cppcoreguidelines-avoid-capturing-lambda-coroutines)
        {
            co_await ulsReader2.Run();
        },
        boost::asio::detached
    );

    boost::asio::post(
        ioContext,
        [&ulsReader, &ulsReader2]() -> void
        {
            std::this_thread::sleep_for(std::chrono::seconds(1)); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            ulsReader.Stop();
            ulsReader2.Stop();
        }
    );

    std::thread ioThread([&ioContext]() -> void
    {
        ioContext.run();
    });

    ioContext.run();
    ioThread.join();
}
