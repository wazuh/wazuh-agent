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

    void start(size_t numThreads) override;
    void stop() override;

    void enqueueTask(std::function<void()> task) override;
    void enqueueTask(boost::asio::awaitable<void> task) override;

private:
    boost::asio::io_context m_ioContext;
    boost::asio::io_context::work m_work;
    std::vector<std::thread> m_threads;
};
