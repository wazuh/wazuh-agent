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
    taskManager.Start(2);

    std::function<void()> task = [this]()
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            taskExecuted = true;
        }
        cv.notify_one();
    };

    taskManager.EnqueueTask(task);

    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]() { return taskExecuted.load(); });
    }

    EXPECT_TRUE(taskExecuted);

    taskManager.Stop();
}

TEST_F(TaskManagerTest, EnqueueCoroutineTask)
{
    taskManager.Start(2);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    auto coroutineTask = [this]() -> boost::asio::awaitable<void>
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            taskExecuted = true;
        }
        cv.notify_one();
        co_return;
    };

    taskManager.EnqueueTask(coroutineTask());

    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]() { return taskExecuted.load(); });
    }

    EXPECT_TRUE(taskExecuted);

    taskManager.Stop();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
