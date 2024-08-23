#pragma once

#include <isignal_handler.hpp>

#include <atomic>
#include <condition_variable>
#include <csignal>
#include <mutex>
#include <vector>

class SignalHandler : public ISignalHandler
{
public:
    SignalHandler(const std::vector<int>& signalsToHandle = {SIGINT, SIGTERM});
    void WaitForSignal() override;

    static void HandleSignal(int signalToHandle);

    static std::atomic<bool> KeepRunning;

private:
    void Initialize(const std::vector<int>& signalsToHandle);

    static std::condition_variable m_cv;
    static std::mutex m_cvMutex;
};
