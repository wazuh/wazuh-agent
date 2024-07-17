#pragma once

#include <functional>

template<typename CoroutineTaskType>
class ITaskManager
{
public:
    virtual ~ITaskManager() = default;

    virtual void start(size_t numThreads) = 0;
    virtual void stop() = 0;

    virtual void enqueueTask(std::function<void()> task) = 0;
    virtual void enqueueTask(CoroutineTaskType task) = 0;
};
