#pragma once

#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace WindowsService
{
    bool InstallService();
    bool RemoveService();
    void SetDispatcherThread();
    void WINAPI ServiceMain();
    void WINAPI ServiceCtrlHandler(DWORD CtrlCode);
    void ServiceStart();
    void ServiceStop();
    void ServiceRestart();
    void ServiceStatus();
} // namespace WindowsService
