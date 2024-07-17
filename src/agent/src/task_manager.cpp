#include <task_manager.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/post.hpp>

TaskManager::TaskManager()
    : m_work(m_ioContext)
{
}

void TaskManager::start(size_t numThreads)
{
    stop();

    for (size_t i = 0; i < numThreads; ++i)
    {
        m_threads.emplace_back([this]() { m_ioContext.run(); });
    }
}

void TaskManager::stop()
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

void TaskManager::enqueueTask(std::function<void()> task)
{
    boost::asio::post(m_ioContext, std::move(task));
}

void TaskManager::enqueueTask(boost::asio::awaitable<void> task)
{
    boost::asio::co_spawn(m_ioContext, std::move(task), boost::asio::detached);
}
