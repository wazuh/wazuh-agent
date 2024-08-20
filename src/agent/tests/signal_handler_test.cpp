#include <gtest/gtest.h>

#include <signal_handler.hpp>

#include <csignal>
#include <future>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{
#ifdef _WIN32
    const DWORD TestSignalToRaise = CTRL_C_EVENT;
    void RaiseSignal()
    {
        SignalHandler::HandleSignal(TestSignalToRaise);
    }
#else
    const int TestSignalToRaise = SIGUSR1;
    void RaiseSignal()
    {
        std::raise(TestSignalToRaise);
    }
#endif
} // namespace

class SignalHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        SignalHandler::KeepRunning.store(true);
    }

    SignalHandler signalHandler = SignalHandler({TestSignalToRaise});
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
    RaiseSignal();
    signalThread.join();

    EXPECT_FALSE(SignalHandler::KeepRunning.load());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
