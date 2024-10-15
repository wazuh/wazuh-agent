#pragma once

#include <gmock/gmock.h>
#include <logcollector.hpp>

using namespace logcollector;

class LogcollectorMock : public Logcollector {
public:
    LogcollectorMock() { }
    MOCK_METHOD(void, SendMessage, (const std::string& location, const std::string& log), (override));
    MOCK_METHOD(void, EnqueueTask, (Awaitable task), (override));
};
