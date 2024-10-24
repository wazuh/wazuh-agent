#include <windows_api_facade.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace windows_api_facade
{
    void* WindowsApiFacade::OpenSCM(unsigned int desiredAccess)
    {
        return ::OpenSCManager(nullptr, nullptr, static_cast<DWORD>(desiredAccess));
    }

    void* WindowsApiFacade::OpenSvc(void* sHandle, std::string sName, unsigned int desiredAccess)
    {
        return ::OpenService(static_cast<SC_HANDLE>(sHandle), sName.c_str(), static_cast<DWORD>(desiredAccess));
    }

    bool WindowsApiFacade::DeleteSvc(void* sHandle)
    {
        return ::DeleteService(static_cast<SC_HANDLE>(sHandle));
    }
} // namespace windows_api_facade
