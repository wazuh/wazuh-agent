#pragma once

#include <gmock/gmock.h>

#include "reader.hpp"
#include "winevt_wrapper_win.hpp"
#include "logcollector.hpp"

namespace logcollector::winevt
{

class MockWinAPIWrapper : public IWinAPIWrapper
{
public:
    MOCK_METHOD(EVT_HANDLE, EvtSubscribe,
        (HANDLE session, HANDLE signalEvent, LPCWSTR channelPath, LPCWSTR query,
         EVT_HANDLE bookmark, PVOID context, EVT_SUBSCRIBE_CALLBACK callback, DWORD flags),
        (override));
    MOCK_METHOD(BOOL, EvtRender, (EVT_HANDLE, EVT_HANDLE, DWORD, DWORD, PVOID, DWORD*, DWORD*),
        (override));
    MOCK_METHOD(BOOL, EvtClose, (EVT_HANDLE), (override));
    void TriggerCallback(EVT_SUBSCRIBE_NOTIFY_ACTION action, EVT_HANDLE event) {
        if (storedCallback) {
            storedCallback(action, storedContext, event);
        }
    }
    EVT_HANDLE StoreCallback(HANDLE, HANDLE, LPCWSTR, LPCWSTR, EVT_HANDLE, PVOID context,
                             EVT_SUBSCRIBE_CALLBACK callback, DWORD) {
        storedContext = context;
        storedCallback = callback;
        return mockSubscriptionHandle;
    }

    EVT_HANDLE mockSubscriptionHandle = reinterpret_cast<EVT_HANDLE>(1);

private:
    PVOID storedContext = nullptr;
    EVT_SUBSCRIBE_CALLBACK storedCallback = nullptr;

};

class LogcollectorMock : public Logcollector {
public:
    LogcollectorMock() {
        ON_CALL(*this, AddReader(::testing::_))
            .WillByDefault(::testing::Invoke([this](std::shared_ptr<IReader> reader) {
                this->Logcollector::AddReader(reader);
            })
        );
    }

    MOCK_METHOD(void, AddReader, (std::shared_ptr<IReader> reader), (override));
    MOCK_METHOD(void, EnqueueTask, (Awaitable task), (override));
    MOCK_METHOD(void, SendMessage, (const std::string& channel, const std::string& message,  const std::string& collectorType), (override));
    boost::asio::awaitable<void> Wait([[maybe_unused]]std::chrono::milliseconds ms)
    {
        co_return;
    }
    void Stop()
    {
        Logcollector::Stop();
    }
};

}
