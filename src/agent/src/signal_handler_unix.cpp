#include <signal_handler.hpp>

void SignalHandler::Initialize(const std::vector<int>& signalsToHandle)
{
    for (const int signal : signalsToHandle)
    {
        std::signal(signal, SignalHandler::HandleSignal);
    }
}
