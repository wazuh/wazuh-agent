#include <gtest/gtest.h>

#include <signal_handler.hpp>

class SignalHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        SignalHandler::KeepRunning.store(true);
    }

    void TearDown() override {}

    SignalHandler signalHandler = SignalHandler({SIGUSR1});
};

TEST_F(SignalHandlerTest, KeepRunningIsTheDefault)
{
    EXPECT_TRUE(SignalHandler::KeepRunning.load());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
