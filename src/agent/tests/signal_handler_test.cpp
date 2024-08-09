#include <gtest/gtest.h>

#include <future>
#include <signal_handler.hpp>
#include <thread>

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

TEST_F(SignalHandlerTest, HandlesSignal)
{
    std::promise<void> readyPromise;
    std::future<void> readyFuture = readyPromise.get_future();
    std::thread signalThread(
        [&]()
        {
            readyPromise.set_value();
            signalHandler.WaitForSignal();
        });

    readyFuture.wait();
    std::raise(SIGUSR1);
    signalThread.join();

    EXPECT_FALSE(SignalHandler::KeepRunning.load());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
