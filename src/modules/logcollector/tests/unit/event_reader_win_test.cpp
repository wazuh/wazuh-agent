#define UNICODE
#include <gtest/gtest.h>

#include <configuration_parser.hpp>
#include "event_reader_win.hpp"
#include <logcollector.hpp>
#include "logcollector_mock.hpp"

#include <sstream>
#include <spdlog/spdlog.h>

class WindowsEventChannel : public ::testing::Test
{
protected:
    void SetUp() override { };

    void GenerateTestEvent()
    {
        HANDLE eventLog = RegisterEventSource(nullptr, L"MyTestSource");

        const wchar_t* message = L"Test Event Content.";
        auto res = ReportEvent(
            eventLog,
            EVENTLOG_INFORMATION_TYPE,
            0,
            1001,
            nullptr,
            1,
            0,
            &message,
            nullptr);

        res = 0;

        DeregisterEventSource(eventLog);
    };

    const std::string channelName {"Application"};
    const std::string query {"TestQuery"};

};

TEST_F(WindowsEventChannel, Constructor)
{
    auto logcollector = LogcollectorMock();
    EXPECT_NO_THROW(WindowsEventTracerReader(logcollector,"","", (time_t) 0 ));
}

TEST_F(WindowsEventChannel, RunMethodEnqueueTask)
{
    auto mockedLogcollector = LogcollectorMock();
    auto reader = std::make_shared<WindowsEventTracerReader>(mockedLogcollector, channelName, query, 5000);

    EXPECT_CALL(mockedLogcollector, EnqueueTask(::testing::_)).Times(1);
    EXPECT_CALL(mockedLogcollector, AddReader(::testing::_));

    mockedLogcollector.AddReader(reader);
}

TEST_F(WindowsEventChannel, AddReader)
{
    auto constexpr CONFIG_RAW = R"(
    logcollector:
      windows:
        - channel: Application
          query: Event[System/EventID = 4624]
        - channel: System
          query: Event[System/EventID = 7040]
    )";

    std::shared_ptr<IReader> capturedReader1;
    std::shared_ptr<IReader> capturedReader2;
    auto mockedLogcollector = LogcollectorMock();
    auto config = std::make_shared<configuration::ConfigurationParser>(std::string(CONFIG_RAW));

    EXPECT_CALL(mockedLogcollector, AddReader(::testing::_)).Times(2)
        .WillOnce(::testing::SaveArg<0>(&capturedReader1))
        .WillOnce(::testing::SaveArg<0>(&capturedReader2));

    AddPlatformSpecificReader(config, mockedLogcollector);

    ASSERT_NE(capturedReader1, nullptr);
    ASSERT_NE(capturedReader2, nullptr);
}

TEST_F(WindowsEventChannel, NoneEvent)
{
    GTEST_SKIP();
}

TEST_F(WindowsEventChannel, SingleEvent)
{
    auto mockedLogcollector = LogcollectorMock();
    auto reader = WindowsEventTracerReader(mockedLogcollector, channelName, query, 5000);
    GenerateTestEvent();
    reader.QueryEvents();
}

TEST_F(WindowsEventChannel, Several)
{
    GTEST_SKIP();
}
