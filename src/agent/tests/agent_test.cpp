#include <agent.hpp>

#include <isignal_handler.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

class MockSignalHandler : public ISignalHandler
{
public:
    MOCK_METHOD(void, WaitForSignal, (), (override));
};

TEST(AgentTests, AgentDefaultConstruction)
{
    EXPECT_NO_THROW(Agent {""});
}

TEST(AgentTests, AgentStopsWhenSignalReceived)
{
    auto mockSignalHandler = std::make_unique<MockSignalHandler>();
    MockSignalHandler* mockSignalHandlerPtr = mockSignalHandler.get();

    EXPECT_CALL(*mockSignalHandlerPtr, WaitForSignal())
        .Times(1)
        .WillOnce([]() { std::this_thread::sleep_for(std::chrono::seconds(1)); });

    Agent agent("", std::move(mockSignalHandler));

    EXPECT_NO_THROW(agent.Run());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
