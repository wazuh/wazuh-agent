#include <restart_handler_win.hpp>

#include <logger.hpp>
#include <shlobj_core.h>

#include <algorithm>
#include <fmt/format.h>

namespace restart_handler
{

    std::vector<char*> RestartHandler::startupCmdLineArgs;

    boost::asio::awaitable<module_command::CommandExecutionResult> RestartForeground()
    {
        STARTUPINFO si = {0};
        PROCESS_INFORMATION pi = {0};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        std::vector<char*> cmdLineArgs;
        std::copy_if(RestartHandler::startupCmdLineArgs.begin(),
                     RestartHandler::startupCmdLineArgs.end(),
                     std::back_inserter(cmdLineArgs),
                     [](char* ptr) { return ptr != nullptr; });

        const std::string cmd = fmt::format("cmd.exe /c timeout /t 3 && {}", fmt::join(cmdLineArgs, " "));

        if (CreateProcess(NULL,
                          (LPSTR)cmd.c_str(),
                          NULL,
                          NULL,
                          FALSE,
                          CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
                          NULL,
                          NULL,
                          &si,
                          &pi))
        {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        LogInfo("Exiting wazuh-agent now");
        GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
        co_return module_command::CommandExecutionResult {module_command::Status::IN_PROGRESS,
                                                          "Pending restart execution"};
    }

    std::string GetPowerShellPath()
    {
        auto is64bits = []() -> bool
        {
            SYSTEM_INFO systemInfo;
            GetNativeSystemInfo(&systemInfo);

            return systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
                   systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64;
        };

        KNOWNFOLDERID folderId = is64bits() ? FOLDERID_System : FOLDERID_SystemX86;

        PWSTR path = nullptr;
        HRESULT hr = SHGetKnownFolderPath(folderId, 0, nullptr, &path);

        if (SUCCEEDED(hr))
        {
            std::wstring powershellPath(path);
            CoTaskMemFree(path);

            powershellPath += L"\\WindowsPowerShell\\v1.0\\powershell.exe";

            int size = WideCharToMultiByte(CP_UTF8, 0, powershellPath.c_str(), -1, nullptr, 0, nullptr, nullptr);
            if (size > 0)
            {
                std::string result(size, 0);
                WideCharToMultiByte(CP_UTF8, 0, powershellPath.c_str(), -1, &result[0], size, nullptr, nullptr);
                return result.c_str(); // returning c_str() removes the trailing null char
            }
        }

        return "powershell";
    }

    boost::asio::awaitable<module_command::CommandExecutionResult> RestartService()
    {
        STARTUPINFO si = {0};
        PROCESS_INFORMATION pi = {0};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        const std::string pwrShell = GetPowerShellPath();

        const std::string cmd = pwrShell + " -Command \"Restart-Service -Name 'wazuh-agent' -Force\"";

        if (CreateProcess(NULL,
                          (LPSTR)cmd.c_str(),
                          NULL,
                          NULL,
                          FALSE,
                          CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
                          NULL,
                          NULL,
                          &si,
                          &pi))
        {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }

        co_return module_command::CommandExecutionResult {module_command::Status::IN_PROGRESS,
                                                          "Pending restart execution"};
    }

    bool RunningAsService()
    {
        return std::any_of(RestartHandler::startupCmdLineArgs.begin(),
                           RestartHandler::startupCmdLineArgs.end(),
                           [](const char* param)
                           { return param != nullptr && std::strcmp(param, "--run-service") == 0; });
    }
} // namespace restart_handler
