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

    /// @copydoc ITaskManager::StartThreadPool
    void StartThreadPool(size_t numThreads) override;

    /// @copydoc ITaskManager::RunSingleThread
    void RunSingleThread() override;

    /// @copydoc ITaskManager::Stop
    void Stop() override;

    /// @copydoc ITaskManager::EnqueueTask
    void EnqueueTask(std::function<void()> task, const std::string& taskID = "") override;

    /// @copydoc ITaskManager::EnqueueTask
    void EnqueueTask(boost::asio::awaitable<void> task, const std::string& taskID = "") override;

    /// @brief Returns the number of tasks enqueued on threads
    /// @return The number of enqueued tasks on threads
    size_t GetNumEnqueuedThreadTasks() const;

private:
    /// @brief The IO context for the task manager
    boost::asio::io_context m_ioContext;

    /// @brief A work guard object to keep the IO context running
    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> m_work;

    /// @brief Threads run by the task manager
    std::vector<std::thread> m_threads;

    /// @brief Number of enqueued threads
    std::atomic<size_t> m_numEnqueuedThreadTasks = 0;

    /// @brief Mutex to control Start and Stop operations
    mutable std::mutex m_mutex;
};
