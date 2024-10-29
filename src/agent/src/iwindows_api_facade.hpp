#pragma once

#include <string>

namespace windows_api_facade
{
    class IWindowsApiFacade
    {
    public:
        virtual ~IWindowsApiFacade() = default;

        virtual void* OpenSCM(unsigned int desiredAccess) const = 0;
        virtual void*
        OpenSvc(void* serviceHandle, const std::string& serviceName, unsigned int desiredAccess) const = 0;
        virtual void*
        CreateSvc(void* serviceHandle, const std::string& serviceName, const std::string& exePath) const = 0;
        virtual bool DeleteSvc(void* serviceHandle) const = 0;
    };
} // namespace windows_api_facade
