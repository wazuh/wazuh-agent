#pragma once

#include <atomic>
#include <csignal>
#include <vector>

class SignalHandler
{
public:
    SignalHandler(const std::vector<int>& signalsToHandle = {SIGINT, SIGTERM});

    static void HandleSignal(int signalToHandle);

    static std::atomic<bool> KeepRunning;
};
