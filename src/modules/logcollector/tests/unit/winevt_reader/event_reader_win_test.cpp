#include <gtest/gtest.h>

#include <configuration_parser.hpp>
#include <event_reader_win.hpp>
#include <logcollector.hpp>

#include "event_reader_mocks.hpp"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <spdlog/spdlog.h>
#include <sstream>

using namespace testing;
using namespace logcollector::winevt;

class WindowsEventChannel : public ::testing::Test
{
protected:
    void SetUp() override {};

    const std::string channelName {"Application"};
    const std::string query {"TestQuery"};
    const std::string collectorType {"eventchannel"};
    const time_t defaultChannelRefresh {50};
};

TEST_F(WindowsEventChannel, Constructor)
{
    auto logcollector = LogcollectorMock();
    EXPECT_NO_THROW(WindowsEventTracerReader(logcollector, "", "", (time_t)0));
}

TEST_F(WindowsEventChannel, RunMethodEnqueueTask)
{
    auto mockedLogcollector = LogcollectorMock();
    auto reader = std::make_shared<WindowsEventTracerReader>(mockedLogcollector, channelName, query, 5000);

    EXPECT_CALL(mockedLogcollector, EnqueueTask(_)).Times(1);
    EXPECT_CALL(mockedLogcollector, AddReader(_));

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

    std::shared_ptr<logcollector::IReader> capturedReader1;
    std::shared_ptr<logcollector::IReader> capturedReader2;
    auto mockedLogcollector = LogcollectorMock();
    auto config = std::make_shared<configuration::ConfigurationParser>(std::string(CONFIG_RAW));

    EXPECT_CALL(mockedLogcollector, AddReader(_))
        .Times(2)
        .WillOnce(SaveArg<0>(&capturedReader1))
        .WillOnce(SaveArg<0>(&capturedReader2));

    mockedLogcollector.AddPlatformSpecificReader(config);

    ASSERT_NE(capturedReader1, nullptr);
    ASSERT_NE(capturedReader2, nullptr);
}
