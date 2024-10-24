#pragma once

#include <windows_api_facade.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace WindowsService
{
    bool InstallService();
    bool RemoveService(windows_api_facade::IWindowsApiFacade& windowsApiFacade);
    void SetDispatcherThread();
    void WINAPI ServiceMain();
    void WINAPI ServiceCtrlHandler(DWORD ctrlCode);
    void ServiceStart();
    void ServiceStop();
    void ServiceRestart();
    void ServiceStatus();
} // namespace WindowsService
