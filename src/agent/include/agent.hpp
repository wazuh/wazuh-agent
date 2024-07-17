#pragma once

#include <task_manager.hpp>

#include <queue>
#include <string>

class Agent
{
public:
    Agent();
    ~Agent();

private:
    std::queue<std::string> m_messageQueue;

    TaskManager m_taskManager;
};
