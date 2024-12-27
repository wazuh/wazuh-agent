#pragma once

#include <itask_manager.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

/// @brief Task manager class
class TaskManager : public ITaskManager<boost::asio::awaitable<void>>
{
public:
    /// @brief Constructor
    TaskManager() = default;

    ~TaskManager() override;

    /// @brief Starts the task manager
    /// @param numThreads The number of threads to start
    void Start(size_t numThreads) override;

    /// @brief Stops the task manager
    void Stop() override;

    /// @brief Enqueues a task to be executed
    /// @param task The task to enqueue
    /// @param taskID The ID of the task
    void EnqueueTask(std::function<void()> task, const std::string& taskID = "") override;

    /// @brief Enqueues a coroutine task to be executed
    /// @param task The coroutine task to enqueue
    /// @param taskID The ID of the task
    void EnqueueTask(boost::asio::awaitable<void> task, const std::string& taskID = "") override;

    /// @brief Returns the number of enqueued threads
    /// @return The number of enqueued threads
    size_t GetNumEnqueuedThreads() const;

private:
    /// @brief The IO context for the task manager
    boost::asio::io_context m_ioContext;

    /// @brief A work guard object to keep the IO context running
    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> m_work;

    /// @brief Threads run by the task manager
    std::vector<std::thread> m_threads;

    /// @brief Number of enqueued threads
    std::atomic<size_t> m_numEnqueuedThreads = 0;

    /// @brief Mutex to control Start and Stop operations
    mutable std::mutex m_mutex;
};
