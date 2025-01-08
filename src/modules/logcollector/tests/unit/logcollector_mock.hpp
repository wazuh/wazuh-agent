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
};

class PushMessageMock {
public:
    MOCK_METHOD(int, Call, (Message), ());
};
