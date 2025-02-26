#pragma once

#include <windows_api_facade.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>

namespace windows_service
{
    /// @brief Installs the Wazuh Agent Service.
    /// @param windowsApiFacade The Windows API facade to use.
    /// @return True if the installation is successful, false otherwise.
    bool InstallService(const windows_api_facade::IWindowsApiFacade& windowsApiFacade);

    /// @brief Removes the Wazuh Agent Service.
    /// @param windowsApiFacade The Windows API facade to use.
    /// @return True if the removal is successful, false otherwise.
    bool RemoveService(const windows_api_facade::IWindowsApiFacade& windowsApiFacade);

    /// @brief Sets the dispatcher thread.
    void SetDispatcherThread();

    /// @brief The service main function.
    /// @param argc The number of command line arguments.
    /// @param argv The command line arguments.
    void WINAPI ServiceMain(DWORD argc, LPSTR* argv);

    /// @brief The service control handler.
    /// @param ctrlCode The control code.
    void WINAPI ServiceCtrlHandler(DWORD ctrlCode);

    /// @brief Starts the Wazuh Agent Service.
    /// @param configFilePath The path to the configuration file.
    void ServiceStart(const std::string& configFilePath);
} // namespace windows_service
