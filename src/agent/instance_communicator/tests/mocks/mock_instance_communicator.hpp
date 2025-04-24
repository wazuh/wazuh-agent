#pragma once

#include <gmock/gmock.h>

#include <iinstance_communicator.hpp>

namespace instance_communicator
{
    class MockInstanceCommunicator : public IInstanceCommunicator
    {
    public:
        MOCK_METHOD(boost::asio::awaitable<void>,
                    Listen,
                    (const std::string& runPath, std::unique_ptr<IListenerWrapper> listenerWrapper),
                    (override));
        MOCK_METHOD(void, Stop, (), (override));
        MOCK_METHOD(void, HandleSignal, (const std::string& signal), (const, override));
    };
} // namespace instance_communicator
