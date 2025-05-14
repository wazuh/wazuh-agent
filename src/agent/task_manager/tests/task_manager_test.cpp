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
    TaskManager taskManager;

    void SetUp() override
    {
        taskExecuted = false;
    }

    std::atomic<bool> taskExecuted;
    std::mutex mtx;
    std::condition_variable cv;
};

TEST_F(TaskManagerTest, StartAndStop)
{
    taskManager.StartThreadPool(4);
    taskManager.Stop();
}

TEST_F(TaskManagerTest, EnqueueFunctionTask)
{
    const std::function<void()> task = [this]()
    {
        taskExecuted = true;
        cv.notify_one();
    };

    taskManager.EnqueueTask(task);

    EXPECT_FALSE(taskExecuted);
    taskManager.StartThreadPool(4);

    while (!taskExecuted.load())
    {
    }

    EXPECT_TRUE(taskExecuted);

    taskManager.Stop();
}

TEST_F(TaskManagerTest, EnqueueCoroutineTask)
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    auto coroutineTask = [this]() -> boost::asio::awaitable<void>
    {
        taskExecuted = true;
        co_return;
    };

    taskManager.EnqueueTask(coroutineTask());

    EXPECT_FALSE(taskExecuted);
    taskManager.StartThreadPool(4);

    while (!taskExecuted.load())
    {
    }

    EXPECT_TRUE(taskExecuted);
    taskManager.Stop();
}

TEST_F(TaskManagerTest, EnqueueFunctionTaskIncrementsCounter)
{
    EXPECT_EQ(taskManager.GetNumEnqueuedThreadTasks(), 0);

    const std::function<void()> task = [this]()
    {
        EXPECT_EQ(taskManager.GetNumEnqueuedThreadTasks(), 1);
        taskExecuted = true;
        cv.notify_one();
    };

    taskManager.EnqueueTask(task);

    EXPECT_FALSE(taskExecuted);
    taskManager.StartThreadPool(4);

    while (!taskExecuted.load())
    {
    }

    EXPECT_TRUE(taskExecuted);

    taskManager.Stop();

    EXPECT_EQ(taskManager.GetNumEnqueuedThreadTasks(), 0);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
