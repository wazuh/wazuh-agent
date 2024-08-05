#include <signal_handler.hpp>

std::atomic<bool> SignalHandler::KeepRunning = true;

SignalHandler::SignalHandler(const std::vector<int>& signalsToHandle)
{
    for (const auto signal : signalsToHandle)
    {
        std::signal(signal, SignalHandler::HandleSignal);
    }
}

void SignalHandler::HandleSignal(int signal)
{
    KeepRunning.store(false);
}
