#pragma once

#include <itask_manager.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>

#include <functional>
#include <thread>
#include <vector>

class TaskManager : public ITaskManager<boost::asio::awaitable<void>>
{
public:
    TaskManager();

    void Start(size_t numThreads) override;
    void Stop() override;

    void EnqueueTask(std::function<void()> task) override;
    void EnqueueTask(boost::asio::awaitable<void> task) override;

private:
    boost::asio::io_context m_ioContext;
    boost::asio::io_context::work m_work;
    std::vector<std::thread> m_threads;
};
