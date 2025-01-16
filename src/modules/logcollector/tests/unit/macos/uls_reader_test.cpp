#include <gtest/gtest.h>

#include <uls_reader.hpp>
#include <logcollector.hpp>

#include <boost/asio.hpp>

#include "../logcollector_mock.hpp"

#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>

class MockIOSLogStoreWrapper : public IOSLogStoreWrapper
{
public:
    MOCK_METHOD(
        std::vector<LogEntry>,
        AllEntries,
        (const double startTimeSeconds, const std::string& query, const LogLevel logLevel),
        (override)
    );
};

TEST(ULSReaderTest, Constructor)
{
    auto logCollector = LogcollectorMock();
    logcollector::ULSReader ulsReader(logCollector);
}

TEST(ULSReaderTest, ConstructorWithStoreWrapper)
{
    auto logCollector = LogcollectorMock();
    auto logStoreMock = std::make_unique<MockIOSLogStoreWrapper>();
    ULSReader reader(std::move(logStoreMock), logCollector);

    // Just ensure the constructor works
    SUCCEED();
}

TEST(ULSReaderTest, RunAndStop)
{
    auto logCollector = LogcollectorMock();
    EXPECT_CALL(logCollector, Wait(::testing::_)).Times(::testing::AnyNumber());

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
    auto logCollector = LogcollectorMock();
    EXPECT_CALL(logCollector, Wait(::testing::_)).Times(::testing::AnyNumber());

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
