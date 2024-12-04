#pragma once

#include <itask_manager.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>

#include <functional>
#include <thread>
#include <vector>

/// @brief Task manager class
class TaskManager : public ITaskManager<boost::asio::awaitable<void>>
{
public:
    /// @brief Constructor
    TaskManager();

    /// @brief Starts the task manager
    /// @param numThreads The number of threads to start
    void Start(size_t numThreads) override;

    /// @brief Stops the task manager
    void Stop() override;

    /// @brief Enqueues a task to be executed
    /// @param task The task to enqueue
    void EnqueueTask(std::function<void()> task) override;

    /// @brief Enqueues a coroutine task to be executed
    /// @param task The coroutine task to enqueue
    void EnqueueTask(boost::asio::awaitable<void> task) override;

    /// @brief Returns the number of enqueued threads
    /// @return The number of enqueued threads
    size_t GetNumEnqueuedThreads() const;

private:
    /// @brief The IO context for the task manager
    boost::asio::io_context m_ioContext;

    /// @brief A work object to keep the IO context running
    boost::asio::io_context::work m_work;

    /// @brief Threads run by the task manager
    std::vector<std::thread> m_threads;

    /// @brief Number of enqueued threads
    size_t m_numEnqueuedThreads = 0;
};
