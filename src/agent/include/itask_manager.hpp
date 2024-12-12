#pragma once

#include <functional>

/// @brief Interface for task managers
///
/// Task managers are responsible for starting and stopping threads and for
/// scheduling tasks to be executed.
///
/// @tparam CoroutineTaskType The type of coroutine tasks that can be scheduled
template<typename CoroutineTaskType>
class ITaskManager
{
public:
    /// @brief Virtual destructor
    virtual ~ITaskManager() = default;

    /// @brief Starts the task manager
    /// @param numThreads The number of threads to start
    virtual void Start(size_t numThreads) = 0;

    /// @brief Stops the task manager
    virtual void Stop() = 0;

    /// @brief Enqueues a task to be executed
    /// @param task The task to enqueue
    virtual void EnqueueTask(std::function<void()> task) = 0;

    /// @brief Enqueues a coroutine task to be executed
    /// @param task The coroutine task to enqueue
    virtual void EnqueueTask(CoroutineTaskType task) = 0;
};
