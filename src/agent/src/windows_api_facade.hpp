#pragma once

#include <iwindows_api_facade.hpp>

namespace windows_api_facade
{
    class WindowsApiFacade : public IWindowsApiFacade
    {
    public:
        void* OpenSCM(unsigned int desiredAccess) const override;
        void* OpenSvc(void* serviceHandle, const std::string& serviceName, unsigned int desiredAccess) const override;
        bool DeleteSvc(void* serviceHandle) const override;
    };
} // namespace windows_api_facade
