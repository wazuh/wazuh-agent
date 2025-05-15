#include <gtest/gtest.h>
#include <task_manager.hpp>

#include <boost/asio.hpp>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

class TaskManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        taskManager = std::make_unique<TaskManager>();
        taskExecuted = false;
    }

    std::unique_ptr<TaskManager> taskManager;
    std::atomic<bool> taskExecuted;
    std::mutex mtx;
    std::condition_variable cv;
};

TEST_F(TaskManagerTest, StopOnUnstartedThreadPool)
{
    taskManager->Stop();
    EXPECT_EQ(taskManager->GetNumThreads(), 0);
}

TEST_F(TaskManagerTest, StartCanBeCalledMultipleTimes)
{
    taskManager->StartThreadPool(4);
    taskManager->StartThreadPool(4);
    taskManager->StartThreadPool(4);
    EXPECT_EQ(taskManager->GetNumThreads(), 4);
}

TEST_F(TaskManagerTest, StopCanBeCalledMultipleTimes)
{
    taskManager->StartThreadPool(4);
    EXPECT_EQ(taskManager->GetNumThreads(), 4);
    taskManager->Stop();
    taskManager->Stop();
    EXPECT_EQ(taskManager->GetNumThreads(), 0);
}

TEST_F(TaskManagerTest, StartAndStop)
{
    taskManager->StartThreadPool(4);
    EXPECT_EQ(taskManager->GetNumThreads(), 4);
    taskManager->Stop();
    EXPECT_EQ(taskManager->GetNumThreads(), 0);
}

TEST_F(TaskManagerTest, IsStoppedReturnsTrueWhenNotStarted)
{
    EXPECT_TRUE(taskManager->IsStopped());
}

TEST_F(TaskManagerTest, IsStoppedReturnsFalseWhenRunning)
{
    taskManager->StartThreadPool(4);
    EXPECT_FALSE(taskManager->IsStopped());
    taskManager->Stop();
    EXPECT_TRUE(taskManager->IsStopped());
}

TEST_F(TaskManagerTest, EnqueueFunctionTask)
{
    const std::function<void()> task = [this]()
    {
        taskExecuted = true;
        cv.notify_one();
    };

    taskManager->EnqueueTask(task);

    EXPECT_FALSE(taskExecuted);
    taskManager->StartThreadPool(4);

    while (!taskExecuted.load())
    {
    }

    EXPECT_TRUE(taskExecuted);

    taskManager->Stop();
}

TEST_F(TaskManagerTest, EnqueueCoroutineTask)
{
    EXPECT_FALSE(taskExecuted);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    auto coroutineTask = [this]() -> boost::asio::awaitable<void>
    {
        taskExecuted = true;
        co_return;
    };

    taskManager->StartThreadPool(4);
    taskManager->EnqueueTask(coroutineTask());

    while (!taskExecuted.load())
    {
    }

    EXPECT_TRUE(taskExecuted);
    taskManager->Stop();
}

TEST_F(TaskManagerTest, EnqueueFunctionTaskIncrementsCounter)
{
    EXPECT_EQ(taskManager->GetNumEnqueuedThreadTasks(), 0);

    const std::function<void()> task = [this]()
    {
        EXPECT_EQ(taskManager->GetNumEnqueuedThreadTasks(), 1);
        taskExecuted = true;
        cv.notify_one();
    };

    taskManager->EnqueueTask(task);

    EXPECT_FALSE(taskExecuted);
    taskManager->StartThreadPool(4);

    while (!taskExecuted.load())
    {
    }

    EXPECT_TRUE(taskExecuted);

    taskManager->Stop();

    EXPECT_EQ(taskManager->GetNumEnqueuedThreadTasks(), 0);
}

TEST_F(TaskManagerTest, RunSingleThreadProcessesFunctionAndThenStops)
{
    taskManager->EnqueueTask([&]() { taskExecuted = true; });
    taskManager->EnqueueTask([&]() { taskManager->Stop(); });
    taskManager->RunSingleThread();

    EXPECT_TRUE(taskExecuted);
    EXPECT_EQ(taskManager->GetNumEnqueuedThreadTasks(), 0);
}

TEST_F(TaskManagerTest, RunSingleThreadProcessesCoroutineAndThenStops)
{
    auto coro = [&]() -> boost::asio::awaitable<void> // NOLINT(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    {
        taskExecuted = true;
        co_return;
    };

    taskManager->EnqueueTask(coro());
    taskManager->EnqueueTask([&]() { taskManager->Stop(); });
    taskManager->RunSingleThread();

    EXPECT_TRUE(taskExecuted);
    EXPECT_EQ(taskManager->GetNumEnqueuedThreadTasks(), 0);
}

TEST_F(TaskManagerTest, IdleRunSingleThreadUnblocksOnExternalStop)
{
    std::atomic<bool> unblocked {false};

    std::thread stopper(
        [&]() // NOLINT(cppcoreguidelines-avoid-capturing-lambda-coroutine)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // NOLINT
            EXPECT_FALSE(taskManager->IsStopped());
            taskManager->Stop();
            unblocked = true;
        });

    taskManager->RunSingleThread();

    stopper.join();
    EXPECT_TRUE(unblocked);
    EXPECT_FALSE(taskExecuted);
    EXPECT_EQ(taskManager->GetNumEnqueuedThreadTasks(), 0);
}

TEST_F(TaskManagerTest, ZeroPoolThenRunSingleThread)
{
    taskManager->StartThreadPool(0);
    taskManager->EnqueueTask([&]() { taskExecuted = true; });
    taskManager->EnqueueTask([&]() { taskManager->Stop(); });
    taskManager->RunSingleThread();

    EXPECT_TRUE(taskExecuted);
    EXPECT_EQ(taskManager->GetNumEnqueuedThreadTasks(), 0);
    taskManager->Stop();
}

TEST_F(TaskManagerTest, StoppingTheTaskManagerCancelsTimer)
{
    taskManager->StartThreadPool(4);
    auto timerTaskExecuted = false;
    auto timerCancelled = true;

    auto task1 = [&]() -> boost::asio::awaitable<void> // NOLINT(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    {
        timerTaskExecuted = true;
        auto timer = taskManager->CreateSteadyTimer(std::chrono::hours(1));
        boost::system::error_code ec;
        co_await timer.async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        timerCancelled = false;
        co_return;
    };
    taskManager->EnqueueTask(task1());

    auto task2 = [&]() -> boost::asio::awaitable<void> // NOLINT(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    {
        taskExecuted = true;
        co_return;
    };
    taskManager->EnqueueTask(task2());

    while (!taskExecuted.load())
    {
    }

    taskManager->Stop();

    EXPECT_TRUE(taskExecuted);
    EXPECT_TRUE(timerTaskExecuted);
    EXPECT_TRUE(timerCancelled);
    EXPECT_EQ(taskManager->GetNumEnqueuedThreadTasks(), 0);
    EXPECT_EQ(taskManager->GetNumThreads(), 0);
}

TEST_F(TaskManagerTest, DestroyingTheTaskManagerCancelsTimer)
{
    taskManager->StartThreadPool(4);
    auto timerTaskExecuted = false;
    auto timerCancelled = true;

    auto task1 = [&]() -> boost::asio::awaitable<void> // NOLINT(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    {
        timerTaskExecuted = true;
        auto timer = taskManager->CreateSteadyTimer(std::chrono::hours(1));
        boost::system::error_code ec;
        co_await timer.async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        timerCancelled = false;
        co_return;
    };
    taskManager->EnqueueTask(task1());

    auto task2 = [&]() -> boost::asio::awaitable<void> // NOLINT(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    {
        taskExecuted = true;
        co_return;
    };
    taskManager->EnqueueTask(task2());

    while (!taskExecuted.load())
    {
    }

    taskManager.reset();

    EXPECT_TRUE(taskExecuted);
    EXPECT_TRUE(timerTaskExecuted);
    EXPECT_TRUE(timerCancelled);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
