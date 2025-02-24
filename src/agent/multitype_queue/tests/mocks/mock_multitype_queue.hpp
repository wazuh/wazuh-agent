#pragma once

#include "gmock/gmock.h"

#include <imultitype_queue.hpp>
#include <message.hpp>

#include <boost/asio/awaitable.hpp>

#include <string>
#include <vector>

class MockMultiTypeQueue : public IMultiTypeQueue
{
public:
    MOCK_METHOD(int, push, (Message message, bool shouldWait), (override));
    MOCK_METHOD(boost::asio::awaitable<int>, pushAwaitable, (Message message), (override));
    MOCK_METHOD(int, push, (std::vector<Message> messages), (override));
    MOCK_METHOD(Message,
                getNext,
                (MessageType type, const std::string moduleName, const std::string moduleType),
                (override));
    MOCK_METHOD(
        boost::asio::awaitable<std::vector<Message>>,
        getNextBytesAwaitable,
        (MessageType type, const size_t messageQuantity, const std::string moduleName, const std::string moduleType),
        (override));
    MOCK_METHOD(
        std::vector<Message>,
        getNextBytes,
        (MessageType type, const size_t messageQuantity, const std::string moduleName, const std::string moduleType),
        (override));
    MOCK_METHOD(bool, pop, (MessageType type, const std::string moduleName, const std::string moduleType), (override));
    MOCK_METHOD(int,
                popN,
                (MessageType type, int messageQuantity, const std::string moduleName, const std::string moduleType),
                (override));
    MOCK_METHOD(bool,
                isEmpty,
                (MessageType type, const std::string moduleName, const std::string moduleType),
                (override));
    MOCK_METHOD(bool,
                isFull,
                (MessageType type, const std::string moduleName, const std::string moduleType),
                (override));
    MOCK_METHOD(int,
                storedItems,
                (MessageType type, const std::string moduleName, const std::string moduleType),
                (override));
    MOCK_METHOD(size_t, sizePerType, (MessageType type), (override));
};
