#include <gtest/gtest.h>

#include <configuration_parser.hpp>
#include <event_reader_win.hpp>
#include <logcollector.hpp>
#include "event_reader_mocks.hpp"
#include "logcollector_mock.hpp"

#include <sstream>
#include <spdlog/spdlog.h>
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

using namespace testing;

class WindowsEventChannel : public ::testing::Test
{
protected:
    void SetUp() override { };

    const std::string channelName {"Application"};
    const std::string query {"TestQuery"};
    const std::string collectorType {"eventchannel"};
    const time_t defaultChannelRefresh {50};
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

    std::shared_ptr<IReader> capturedReader1;
    std::shared_ptr<IReader> capturedReader2;
    auto mockedLogcollector = LogcollectorMock();
    auto config = std::make_shared<configuration::ConfigurationParser>(std::string(CONFIG_RAW));

    EXPECT_CALL(mockedLogcollector, AddReader(_)).Times(2)
        .WillOnce(SaveArg<0>(&capturedReader1))
        .WillOnce(SaveArg<0>(&capturedReader2));

    AddPlatformSpecificReader(config, mockedLogcollector);

    ASSERT_NE(capturedReader1, nullptr);
    ASSERT_NE(capturedReader2, nullptr);
}

TEST_F(WindowsEventChannel, RunMethodFailedSubscription)
{
    auto mockedWinAPI = std::make_shared<MockWinAPIWrapper>();
    auto mockedLogcollector = LogcollectorMock();

    auto reader = std::make_shared<WindowsEventTracerReader>(
        mockedLogcollector, channelName, query, defaultChannelRefresh, mockedWinAPI);

    EVT_HANDLE mockHandle = reinterpret_cast<EVT_HANDLE>(0);
    EXPECT_CALL(*mockedWinAPI, EvtSubscribe(_, _, _, _,_, _, _, _))
        .WillOnce(Return(mockHandle));

    auto task = reader->QueryEvents();
    boost::asio::io_context ioContext;
    boost::asio::co_spawn(ioContext, std::move(task), boost::asio::detached);

    EXPECT_NO_THROW(ioContext.run());
}

TEST_F(WindowsEventChannel, RunMethodSuccessfulSubscription)
{
    auto mockedWinAPI = std::make_shared<MockWinAPIWrapper>();
    auto mockedLogcollector = LogcollectorMock();

    auto reader = std::make_shared<WindowsEventTracerReader>(
        mockedLogcollector, channelName, query, defaultChannelRefresh, mockedWinAPI);

    EXPECT_CALL(*mockedWinAPI, EvtSubscribe(_, _, _, _, _, _, _, _))
        .WillOnce([&mockedWinAPI](HANDLE session, HANDLE signalEvent, LPCWSTR channelPath,
                                  LPCWSTR query, EVT_HANDLE bookmark, PVOID context,
                                  EVT_SUBSCRIBE_CALLBACK callback, DWORD flags) {
            return mockedWinAPI->StoreCallback(session, signalEvent, channelPath, query, bookmark,
                                               context, callback, flags);
        });

    EXPECT_CALL(*mockedWinAPI, EvtClose(mockedWinAPI->mockSubscriptionHandle))
        .WillOnce(Return(TRUE));

    auto task = reader->QueryEvents();
    boost::asio::io_context ioContext;
    boost::asio::co_spawn(ioContext, std::move(task), boost::asio::detached);

    EVT_HANDLE mockEventHandle = reinterpret_cast<EVT_HANDLE>(2);
    mockedWinAPI->TriggerCallback(EvtSubscribeActionDeliver, mockEventHandle);

    reader->Stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_NO_THROW(ioContext.run());
}

TEST_F(WindowsEventChannel, ProcessEvent_SendsMessage)
{
    //TODO:
    GTEST_SKIP();
    // Arrange: Create mocks
    auto mockedWinAPI = std::make_shared<MockWinAPIWrapper>();
    auto mockedLogcollector = LogcollectorMock();

    auto reader = std::make_shared<WindowsEventTracerReader>(
        mockedLogcollector, channelName, query, defaultChannelRefresh, mockedWinAPI);

    // Mock EvtRender to return a fake event XML string
    std::wstring fakeEventXml = L"<Event><Message>Test Log</Message></Event>";
    DWORD fakeBufferUsed = static_cast<DWORD>((fakeEventXml.size() + 1) * sizeof(wchar_t)); // Include null terminator

    EXPECT_CALL(*mockedWinAPI, EvtRender(_, _, EvtRenderEventXml, 0, nullptr, _, _))
        .WillOnce(DoAll(SetArgPointee<5>(fakeBufferUsed), Return(TRUE)));

    EXPECT_CALL(*mockedWinAPI, EvtRender(_, _, EvtRenderEventXml, fakeBufferUsed, _, _, _))
        .WillOnce(DoAll(Invoke([&fakeEventXml](EVT_HANDLE, EVT_HANDLE, DWORD, DWORD bufferSize, void* buffer, DWORD*, DWORD*) {
                          std::memcpy(buffer, fakeEventXml.data(), bufferSize);
                      }),
                      Return(TRUE)));

    // Expect the SendMessage method to be called with the processed log
    // EXPECT_CALL(*mockedLogcollector, SendMessage(channelName, "Test Log", _))
    //     .Times(1);

    // Act: Simulate an event
    EVT_HANDLE mockEventHandle = reinterpret_cast<EVT_HANDLE>(2);
    reader->ProcessEvent(mockEventHandle);
}
