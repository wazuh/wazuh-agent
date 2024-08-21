#include <signal_handler.hpp>

#include <windows.h>

void SignalHandler::Initialize([[maybe_unused]] const std::vector<int>& signalsToHandle)
{
    SetConsoleCtrlHandler(
        [](DWORD dwCtrlType) -> BOOL
        {
            if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT)
            {
                SignalHandler::HandleSignal(dwCtrlType);
                return TRUE;
            }
            return FALSE;
        },
        TRUE);
}
