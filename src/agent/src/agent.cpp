#include <agent.hpp>
#include <message_task.hpp>

#include <chrono>
#include <thread>

Agent::Agent()
{
    m_taskManager.start(std::thread::hardware_concurrency());
    m_taskManager.enqueueTask(StatefulMessageProcessingTask(m_messageQueue));
    m_taskManager.enqueueTask(StatelessMessageProcessingTask(m_messageQueue));
}

Agent::~Agent()
{
    // Sleep for 2 seconds
    std::this_thread::sleep_for(std::chrono::seconds(2));
    m_taskManager.stop();
}
