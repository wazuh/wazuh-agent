#include <windows_service.hpp>

#include <logger.hpp>

#include <chrono>
#include <thread>

namespace
{
    SERVICE_STATUS g_ServiceStatus = {};
    SERVICE_STATUS_HANDLE g_StatusHandle = nullptr;
    HANDLE g_ServiceStopEvent = INVALID_HANDLE_VALUE;
    SERVICE_DESCRIPTION g_serviceDescription;
    const std::string AGENT_SERVICENAME = "Wazuh Agent";
    const std::string AGENT_SERVICEDESCRIPTION = "Wazuh Windows Agent";

    std::string GetExecutablePath()
    {
        char buffer[MAX_PATH];
        GetModuleFileName(NULL, buffer, MAX_PATH);
        return std::string(buffer);
    }
} // namespace

namespace WindowsService
{
    bool InstallService()
    {
        const std::string exePath = GetExecutablePath() + " service";

        SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
        if (!schSCManager)
        {
            LogError("OpenSCManager fail: {}", GetLastError());
            return false;
        }

        SC_HANDLE schService = CreateService(schSCManager,
                                             AGENT_SERVICENAME.c_str(),
                                             AGENT_SERVICENAME.c_str(),
                                             SERVICE_ALL_ACCESS,
                                             SERVICE_WIN32_OWN_PROCESS,
                                             SERVICE_AUTO_START,
                                             SERVICE_ERROR_NORMAL,
                                             exePath.c_str(),
                                             nullptr,
                                             nullptr,
                                             nullptr,
                                             nullptr,
                                             nullptr);

        if (!schService)
        {
            LogError("CreateService fail: {}", GetLastError());
            CloseServiceHandle(schSCManager);
            return false;
        }

        g_serviceDescription.lpDescription = const_cast<char*>(AGENT_SERVICEDESCRIPTION.c_str());
        ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &g_serviceDescription);

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        LogInfo("Wazuh Agent Service successfully installed.");

        return true;
    }

    bool RemoveService()
    {
        SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
        if (!schSCManager)
        {
            LogError("OpenSCManager fail: {}", GetLastError());
            return false;
        }

        SC_HANDLE schService = OpenService(schSCManager, AGENT_SERVICENAME.c_str(), DELETE);
        if (!schService)
        {
            LogError("OpenService fail: {}", GetLastError());
            CloseServiceHandle(schSCManager);
            return false;
        }

        if (!DeleteService(schService))
        {
            LogError("DeleteService fail: {}", GetLastError());
            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return false;
        }

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

        LogInfo("Wazuh Agent Service successfully removed.");

        return true;
    }

    void SetDispatcherThread()
    {
        SERVICE_TABLE_ENTRY ServiceTable[] = {{(LPSTR)AGENT_SERVICENAME.c_str(), (LPSERVICE_MAIN_FUNCTION)ServiceMain},
                                              {NULL, NULL}};

        if (!StartServiceCtrlDispatcher(ServiceTable))
        {
            LogError("Error: StartServiceCtrlDispatcher {}", GetLastError());
        }
    }

    void WINAPI ServiceMain()
    {
        g_StatusHandle = RegisterServiceCtrlHandler(AGENT_SERVICENAME.c_str(), ServiceCtrlHandler);

        if (!g_StatusHandle)
        {
            LogError("Failed to register ServiceCtrlHandler. Error: {}", GetLastError());
            return;
        }

        g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
        g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PARAMCHANGE;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwServiceSpecificExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 0;

        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

        g_ServiceStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        if (!g_ServiceStopEvent)
        {
            LogError("Failed to create stop event. Error: {}", GetLastError());
            g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
            SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
            return;
        }

        g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

        LogInfo("Starting Wazuh Agent.");
        while (true)
        {
            LogInfo("Agent is still running in the registration loop...");
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }

        WaitForSingleObject(g_ServiceStopEvent, INFINITE);

        CloseHandle(g_ServiceStopEvent);
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
    }

    void WINAPI ServiceCtrlHandler(DWORD CtrlCode)
    {
        switch (CtrlCode)
        {
            case SERVICE_CONTROL_STOP:
                // TO DO
                break;
            case SERVICE_CONTROL_SHUTDOWN:
                // TO DO
                break;
            case SERVICE_CONTROL_PARAMCHANGE:
                // TO DO
                break;
            default: break;
        }
    }

} // namespace WindowsService
