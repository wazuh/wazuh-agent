#include <task_manager.hpp>

#include <logger.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/post.hpp>

#include <utility>

TaskManager::~TaskManager()
{
    Stop();
}

void TaskManager::Start(size_t numThreads)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_work || !m_threads.empty())
    {
        LogError("Task manager already started");
        return;
    }

    m_work = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(
        boost::asio::make_work_guard(m_ioContext));

    for (size_t i = 0; i < numThreads; ++i)
    {
        m_threads.emplace_back([this]() { m_ioContext.run(); });
    }
}

void TaskManager::Stop()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_work)
    {
        m_work.reset();
    }

    if (!m_ioContext.stopped())
    {
        m_ioContext.stop();
    }

    if (!m_threads.empty())
    {
        for (std::thread& thread : m_threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
        m_threads.clear();
        m_numEnqueuedThreads = 0;
    }

    m_ioContext.reset();
}

void TaskManager::EnqueueTask(std::function<void()> task, const std::string& taskID)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (++m_numEnqueuedThreads > m_threads.size())
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
            LogError("{} task exited with an exception: {}", taskID.empty() ? "Anonymous" : taskID, e.what());
        }
        --m_numEnqueuedThreads;
    };

    boost::asio::post(m_ioContext, std::move(taskWithThreadCounter));
}

void TaskManager::EnqueueTask(boost::asio::awaitable<void> task, const std::string& taskID)
{
    std::lock_guard<std::mutex> lock(m_mutex);

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
                LogError("{} coroutine task exited with an exception: {}",
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
