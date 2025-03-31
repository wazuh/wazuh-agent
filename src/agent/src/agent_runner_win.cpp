#include <agent_runner.hpp>

#include <windows_service.hpp>

namespace
{
    /// Command-line options
    static const auto OPT_INSTALL_SERVICE {"install-service"};
    static const auto OPT_INSTALL_SERVICE_DESC {"Use this option to install Wazuh as a Windows service"};
    static const auto OPT_REMOVE_SERVICE {"remove-service"};
    static const auto OPT_REMOVE_SERVICE_DESC {"Use this option to remove Wazuh Windows service"};
    static const auto OPT_RUN_SERVICE {"run-service"};
    static const auto OPT_RUN_SERVICE_DESC {"Use this option to run Wazuh as a Windows service"};
} // namespace

void AgentRunner::AddPlatformSpecificOptions()
{
    // clang-format off
    m_generalOptions.add_options()
        (OPT_INSTALL_SERVICE, OPT_INSTALL_SERVICE_DESC)
        (OPT_REMOVE_SERVICE, OPT_REMOVE_SERVICE_DESC)
        (OPT_RUN_SERVICE, OPT_RUN_SERVICE_DESC);
    // clang-format on
}

std::optional<int> AgentRunner::HandlePlatformSpecificOptions() const
{
    if (m_options.count(OPT_INSTALL_SERVICE))
    {
        windows_api_facade::WindowsApiFacade windowsApiFacade;
        return windows_service::InstallService(windowsApiFacade) ? 0 : 1;
    }
    else if (m_options.count(OPT_REMOVE_SERVICE))
    {
        windows_api_facade::WindowsApiFacade windowsApiFacade;
        return windows_service::RemoveService(windowsApiFacade) ? 0 : 1;
    }
    else if (m_options.count(OPT_RUN_SERVICE))
    {
        windows_service::SetDispatcherThread();
    }
    return std::nullopt;
}

bool SendSignal(const std::string& message)
{
    HANDLE hPipe = CreateFile(pipeName.c_str(), // pipe name
                              GENERIC_WRITE,    // write access
                              0,                // no sharing
                              NULL,             // default security attributes
                              OPEN_EXISTING,    // opens existing pipe
                              0,                // default attributes
                              NULL);            // no template file

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Client connect error: " << GetLastError() << "\n";
        return false;
    }

    std::string command = message + "\n";
    DWORD bytesWritten = 0;

    BOOL success = WriteFile(hPipe,                              // pipe handle
                             command.c_str(),                    // message
                             static_cast<DWORD>(command.size()), // message length
                             &bytesWritten,                      // bytes written
                             NULL);                              // not overlapped

    if (!success || bytesWritten != command.size())
    {
        std::cerr << "Client write error: " << GetLastError() << "\n";
        CloseHandle(hPipe);
        return false;
    }

    std::cout << "Client sent signal: " << message << "\n";

    // Done writing: close the handle to signal EOF
    CloseHandle(hPipe);
    return true;
}
