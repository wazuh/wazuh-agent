#pragma once

#include <windows_api_facade.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace WindowsService
{
    bool InstallService(const windows_api_facade::IWindowsApiFacade& windowsApiFacade);
    bool RemoveService(const windows_api_facade::IWindowsApiFacade& windowsApiFacade);
    void SetDispatcherThread();
    void WINAPI ServiceMain(DWORD argc, LPSTR* argv);
    void WINAPI ServiceCtrlHandler(DWORD ctrlCode);
    void ServiceStart(const std::string& configFile);
    void ServiceStop();
    void ServiceRestart(const std::string& configFile);
    void ServiceStatus();
} // namespace WindowsService
