#include <task_manager.hpp>

#include <logger.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/post.hpp>

#include <memory>
#include <utility>

TaskManager::TaskManager()
    : m_work(m_ioContext)
{
}

void TaskManager::Start(size_t numThreads)
{
    Stop();

    for (size_t i = 0; i < numThreads; ++i)
    {
        m_threads.emplace_back([this]() { m_ioContext.run(); });
    }
}

void TaskManager::Stop()
{
    m_ioContext.stop();

    for (std::thread& thread : m_threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    m_threads.clear();
    m_ioContext.reset();
}

void TaskManager::EnqueueTask(std::function<void()> task, const std::string& taskID)
{
    if (++m_numEnqueuedThreads > m_threads.size() - 1) // -1 to account for the coroutines
    {
        LogError("Enqueued more threaded tasks than available threads");
    }

    auto taskWithThreadCounter = [this, taskID, task = std::move(task)]() mutable
    {
        try
        {
            task();
        }
        catch (const std::exception& e)
        {
            LogError("{} task threw an exception: {}", taskID.empty() ? "Anonymous" : taskID, e.what());
        }
        --m_numEnqueuedThreads;
    };

    boost::asio::post(m_ioContext, std::move(taskWithThreadCounter));
}

void TaskManager::EnqueueTask(boost::asio::awaitable<void> task, const std::string& taskID)
{
    // NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    boost::asio::co_spawn(
        m_ioContext,
        [sharedData = std::make_shared<std::pair<std::string, boost::asio::awaitable<void>>>(
             taskID, std::move(task))]() mutable -> boost::asio::awaitable<void>
        {
            try
            {
                co_await std::move(sharedData->second);
            }
            catch (const std::exception& e)
            {
                LogError("{} coroutine task threw an exception: {}",
                         sharedData->first.empty() ? "Anonymous" : sharedData->first,
                         e.what());
            }
        },
        boost::asio::detached);
    // NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)
}

size_t TaskManager::GetNumEnqueuedThreads() const
{
    return m_numEnqueuedThreads;
}
