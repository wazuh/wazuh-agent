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
    taskManager.Start(2);
    taskManager.Stop();
}

TEST_F(TaskManagerTest, EnqueueFunctionTask)
{
    std::function<void()> task = [this]()
    {
        taskExecuted = true;
        cv.notify_one();
    };

    taskManager.EnqueueTask(task);

    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock,
                [&]()
                {
                    taskManager.Start(2);
                    return taskExecuted.load();
                });
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
        cv.notify_one();
        co_return;
    };

    taskManager.EnqueueTask(coroutineTask());

    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock,
                [&]()
                {
                    taskManager.Start(2);
                    return taskExecuted.load();
                });
    }

    EXPECT_TRUE(taskExecuted);

    taskManager.Stop();
}

TEST_F(TaskManagerTest, EnqueueFunctionTaskIncrementsCounter)
{
    EXPECT_EQ(taskManager.GetNumEnqueuedThreads(), 0);

    std::function<void()> task = [this]()
    {
        EXPECT_EQ(taskManager.GetNumEnqueuedThreads(), 1);
        taskExecuted = true;
        cv.notify_one();
    };

    taskManager.EnqueueTask(task);

    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock,
                [&]()
                {
                    taskManager.Start(2);
                    return taskExecuted.load();
                });
    }

    taskManager.Stop();

    EXPECT_TRUE(taskExecuted);
    EXPECT_EQ(taskManager.GetNumEnqueuedThreads(), 0);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
