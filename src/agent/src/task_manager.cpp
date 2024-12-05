#include <task_manager.hpp>

#include <logger.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/post.hpp>

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

void TaskManager::EnqueueTask(std::function<void()> task)
{
    if (++m_numEnqueuedThreads > m_threads.size() - 1) // -1 to account for the coroutines
    {
        LogError("Enqueued more threaded tasks than available threads");
    }

    auto taskWithThreadCounter = [this, task = std::move(task)]() mutable
    {
        try
        {
            task();
        }
        catch (const std::exception& e)
        {
            LogError("Task threw an exception: {}", e.what());
        }
        --m_numEnqueuedThreads;
    };

    boost::asio::post(m_ioContext, std::move(taskWithThreadCounter));
}

void TaskManager::EnqueueTask(boost::asio::awaitable<void> task)
{
    boost::asio::co_spawn(
        m_ioContext,
        [task = std::move(task)]() mutable -> boost::asio::awaitable<void>
        {
            try
            {
                co_await std::move(task);
            }
            catch (const std::exception& e)
            {
                LogError("Coroutine task threw an exception: {}", e.what());
            }
        },
        boost::asio::detached);
}

size_t TaskManager::GetNumEnqueuedThreads() const
{
    return m_numEnqueuedThreads;
}
