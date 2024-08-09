#pragma once

#include <functional>

template<typename CoroutineTaskType>
class ITaskManager
{
public:
    virtual ~ITaskManager() = default;

    virtual void Start(size_t numThreads) = 0;
    virtual void Stop() = 0;

    virtual void EnqueueTask(std::function<void()> task) = 0;
    virtual void EnqueueTask(CoroutineTaskType task) = 0;
};
