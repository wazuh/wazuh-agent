#include <signal_handler.hpp>

std::atomic<bool> SignalHandler::KeepRunning = true;
std::condition_variable SignalHandler::m_cv;
std::mutex SignalHandler::m_cvMutex;

SignalHandler::SignalHandler(const std::vector<int>& signalsToHandle)
{
    Initialize(signalsToHandle);
}

void SignalHandler::HandleSignal(int signal)
{
    KeepRunning.store(false);
    m_cv.notify_one();
}

void SignalHandler::WaitForSignal()
{
    std::unique_lock<std::mutex> lock(m_cvMutex);
    m_cv.wait(lock, [] { return !KeepRunning.load(); });
}
