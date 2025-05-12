#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <macos_reader.hpp>
#include <reader.hpp>

#include <boost/asio.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace
{
    const auto DUMMY_PUSH = [](const std::string&, const std::string&, const std::string&) {
    };
    const auto DUMMY_WAIT = [](std::chrono::milliseconds) -> logcollector::Awaitable
    {
        co_return;
    };
} // namespace

namespace macos_reader_tests
{
    constexpr time_t DEFAULT_WAIT_IN_MILLIS = 50;

    using namespace logcollector;
    using ::testing::_; // NOLINT(bugprone-reserved-identifier)
    using ::testing::Invoke;
    using ::testing::Return;

    class MockIOSLogStoreWrapper : public IOSLogStoreWrapper
    {
    public:
        MOCK_METHOD(std::vector<LogEntry>,
                    AllEntries,
                    (const double startTimeSeconds, const std::string& query, const LogLevel logLevel),
                    (override));
    };

    TEST(MacOSReaderTest, Constructor)
    {
        const logcollector::MacOSReader macOSReader(DUMMY_PUSH, DUMMY_WAIT, DEFAULT_WAIT_IN_MILLIS);
    }

    TEST(MacOSReaderTest, ConstructorWithStoreWrapper)
    {
        auto logStoreMock = std::make_unique<MockIOSLogStoreWrapper>();
        const MacOSReader reader(std::move(logStoreMock), DUMMY_PUSH, DUMMY_WAIT, DEFAULT_WAIT_IN_MILLIS);

        // Just ensure the constructor works
        SUCCEED();
    }

    TEST(MacOSReaderTest, RunAndStop)
    {
        boost::asio::io_context ioContext;

        auto didMacOSReaderRun = false;

        logcollector::MacOSReader macOSReader(DUMMY_PUSH, DUMMY_WAIT, DEFAULT_WAIT_IN_MILLIS);
        boost::asio::co_spawn(
            ioContext,
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines)
            [&macOSReader, &didMacOSReaderRun]() -> boost::asio::awaitable<void>
            {
                co_await macOSReader.Run();
                didMacOSReaderRun = true;
            },
            boost::asio::detached);

        boost::asio::post(ioContext,
                          [&macOSReader]() -> void
                          {
                              const auto sleepTime = std::chrono::seconds(1);
                              std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
                              macOSReader.Stop();
                          });

        std::thread ioThread([&ioContext]() -> void { ioContext.run(); });

        ioContext.run();

        EXPECT_TRUE(didMacOSReaderRun);

        ioThread.join();
    }

    TEST(MacOSReaderTest, CreateQueryFromRule)
    {
        logcollector::MacOSReader macOSReader(DUMMY_PUSH,
                                              DUMMY_WAIT,
                                              DEFAULT_WAIT_IN_MILLIS,
                                              "debug",
                                              R"(process != "sshd" OR message CONTAINS "invalid")",
                                              {"trace", "activity", "log"});

        logcollector::MacOSReader macOSReader2(DUMMY_PUSH,
                                               DUMMY_WAIT,
                                               DEFAULT_WAIT_IN_MILLIS,
                                               "debug",
                                               R"(process != "wazuh-agent"")",
                                               {"trace", "activity", "log"});

        boost::asio::io_context ioContext;

        boost::asio::co_spawn(
            ioContext,
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines)
            [&macOSReader]() -> boost::asio::awaitable<void> { co_await macOSReader.Run(); },
            boost::asio::detached);

        boost::asio::co_spawn(
            ioContext,
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines)
            [&macOSReader2]() -> boost::asio::awaitable<void> { co_await macOSReader2.Run(); },
            boost::asio::detached);

        boost::asio::post(ioContext,
                          [&macOSReader, &macOSReader2]() -> void
                          {
                              std::this_thread::sleep_for(std::chrono::seconds(1));
                              macOSReader.Stop();
                              macOSReader2.Stop();
                          });

        std::thread ioThread([&ioContext]() -> void { ioContext.run(); });

        ioContext.run();
        ioThread.join();
    }

    TEST(MacOSReaderTest, ReadsEntriesCorrectly)
    {
        const double aPointInTime = 1672531200.0;
        const double anotherPointInTime = 1672531260.0;

        auto logStoreMock = std::make_unique<MockIOSLogStoreWrapper>();
        EXPECT_CALL(*logStoreMock, AllEntries(_, _, _))
            .WillOnce(Return(std::vector<IOSLogStoreWrapper::LogEntry> {
                {aPointInTime, "2023-01-01T00:00:00Z", "Sample log message 1"},
                {anotherPointInTime, "2023-01-01T00:01:00Z", "Sample log message 2"}}))
            .WillRepeatedly(Return(std::vector<IOSLogStoreWrapper::LogEntry> {}));

        std::unique_ptr<MacOSReader> macOSReader;

        const auto dummyPush = [&macOSReader](const std::string&, const std::string& dumpedData, const std::string&)
        {
            EXPECT_THAT(dumpedData, ::testing::HasSubstr("2023-01-01T00"));
            EXPECT_THAT(dumpedData, ::testing::HasSubstr("Sample log message "));
            macOSReader->Stop();
        };

        macOSReader =
            std::make_unique<MacOSReader>(std::move(logStoreMock), dummyPush, DUMMY_WAIT, DEFAULT_WAIT_IN_MILLIS);

        boost::asio::io_context ioContext;

        boost::asio::co_spawn(
            ioContext,
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines)
            [&macOSReader]() -> boost::asio::awaitable<void> { co_await macOSReader->Run(); },
            boost::asio::detached);

        ioContext.run();
    }

} // namespace macos_reader_tests
