#include <signal_handler.hpp>

#include <csignal>

void SignalHandler::Initialize(const std::vector<int>& signalsToHandle)
{
    for (int signal : signalsToHandle)
    {
        std::signal(signal, SignalHandler::HandleSignal);
    }
}
