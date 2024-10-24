#pragma once

#include <iwindows_api_facade.hpp>

namespace windows_api_facade
{
    class WindowsApiFacade : public IWindowsApiFacade
    {
    public:
        void* OpenSCM(unsigned int desiredAccess) override;
        void* OpenSvc(void* sHandle, std::string sName, unsigned int desiredAccess) override;
        bool DeleteSvc(void* sHandle) override;
    };
} // namespace windows_api_facade
