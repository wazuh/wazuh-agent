#pragma once

#include <string>

namespace windows_api_facade
{
    class IWindowsApiFacade
    {
    public:
        virtual ~IWindowsApiFacade() = default;

        virtual void* OpenSCM(unsigned int desiredAccess) = 0;
        virtual void* OpenSvc(void* sHandle, std::string sName, unsigned int desiredAccess) = 0;
        virtual bool DeleteSvc(void* sHandle) = 0;
    };
} // namespace windows_api_facade
