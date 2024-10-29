#include <windows_api_facade.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace windows_api_facade
{
    void* WindowsApiFacade::OpenSCM(unsigned int desiredAccess) const
    {
        return ::OpenSCManager(nullptr, nullptr, static_cast<DWORD>(desiredAccess));
    }

    void*
    WindowsApiFacade::OpenSvc(void* serviceHandle, const std::string& serviceName, unsigned int desiredAccess) const
    {
        return ::OpenService(
            static_cast<SC_HANDLE>(serviceHandle), serviceName.c_str(), static_cast<DWORD>(desiredAccess));
    }

    void*
    WindowsApiFacade::CreateSvc(void* serviceHandle, const std::string& serviceName, const std::string& exePath) const
    {
        return ::CreateService(static_cast<SC_HANDLE>(serviceHandle),
                               serviceName.c_str(),
                               serviceName.c_str(),
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
    }

    bool WindowsApiFacade::DeleteSvc(void* serviceHandle) const
    {
        return ::DeleteService(static_cast<SC_HANDLE>(serviceHandle));
    }
} // namespace windows_api_facade
