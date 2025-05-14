#include <gtest/gtest.h>
#include <task_manager.hpp>

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
}

TEST_F(TaskManagerTest, StartCanBeCalledMultipleTimes)
{
    taskManager->StartThreadPool(4);
    taskManager->StartThreadPool(4);
    taskManager->StartThreadPool(4);
}

TEST_F(TaskManagerTest, StopCanBeCalledMultipleTimes)
{
    taskManager->StartThreadPool(4);
    taskManager->Stop();
    taskManager->Stop();
}

TEST_F(TaskManagerTest, StartAndStop)
{
    taskManager->StartThreadPool(4);
    taskManager->Stop();
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

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
