#pragma once

#include <gmock/gmock.h>
#include <logcollector.hpp>
#include <reader.hpp>

using namespace logcollector;

class LogcollectorMock : public Logcollector {
public:
    LogcollectorMock() {
        ON_CALL(*this, AddReader(::testing::_))
            .WillByDefault(::testing::Invoke([this](std::shared_ptr<IReader> reader) {
                this->Logcollector::AddReader(reader);
            })
        );
    }

    void SetupFileReader(std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
    {
        Logcollector::SetupFileReader(configurationParser);
    }
    MOCK_METHOD(void, AddReader, (std::shared_ptr<IReader> reader), (override));
    MOCK_METHOD(void, EnqueueTask, (Awaitable task), (override));
    // TODO:
    // MOCK_METHOD(void, SendMessage, (const std::string& channel, const std::string& message, int collectorType), ());
    boost::asio::awaitable<void> Wait([[maybe_unused]]std::chrono::milliseconds ms)
    {
        return Logcollector::Wait(ms);
    }
    void Stop()
    {
        Logcollector::Stop();
    }
};

class PushMessageMock {
public:
    MOCK_METHOD(int, Call, (Message), ());
};
