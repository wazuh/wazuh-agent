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
    taskManager.start(2);
    taskManager.stop();
}

TEST_F(TaskManagerTest, EnqueueFunctionTask)
{
    taskManager.start(1);

    std::atomic<int> counter = 0;
    std::function<void()> task = [&counter]()
    {
        ++counter;
    };

    taskManager.enqueueTask(task);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(counter, 1);

    taskManager.stop();
}

TEST_F(TaskManagerTest, EnqueueCoroutineTask)
{
    taskManager.start(1);

    std::atomic<int> counter = 0;
    auto coroutineTask = [&counter]() -> boost::asio::awaitable<void>
    {
        ++counter;
        co_return;
    };

    taskManager.enqueueTask(coroutineTask());

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(counter, 1);

    taskManager.stop();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
