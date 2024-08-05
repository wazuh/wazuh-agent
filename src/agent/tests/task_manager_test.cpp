#include <gtest/gtest.h>
#include <task_manager.hpp>

#include <atomic>
#include <chrono>

class TaskManagerTest : public ::testing::Test
{
protected:
    TaskManager taskManager;

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(TaskManagerTest, StartAndStop)
{
    taskManager.Start(2);
    taskManager.Stop();
}

TEST_F(TaskManagerTest, EnqueueFunctionTask)
{
    taskManager.Start(1);

    std::atomic<int> counter = 0;
    std::function<void()> task = [&counter]()
    {
        ++counter;
    };

    taskManager.EnqueueTask(task);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(counter, 1);

    taskManager.Stop();
}

TEST_F(TaskManagerTest, EnqueueCoroutineTask)
{
    taskManager.Start(1);

    std::atomic<int> counter = 0;
    auto coroutineTask = [&counter]() -> boost::asio::awaitable<void>
    {
        ++counter;
        co_return;
    };

    taskManager.EnqueueTask(coroutineTask());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(counter, 1);

    taskManager.Stop();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
