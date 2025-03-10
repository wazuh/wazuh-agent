#pragma once

#include <iwindows_api_facade.hpp>

namespace windows_api_facade
{
    /// @brief Class that provides an interface to Windows API for service management.
    class WindowsApiFacade : public IWindowsApiFacade
    {
    public:
        /// @copydoc IWindowsApiFacade::OpenSCM
        void* OpenSCM(unsigned int desiredAccess) const override;

        /// @copydoc IWindowsApiFacade::OpenSvc
        void* OpenSvc(void* serviceHandle, const std::string& serviceName, unsigned int desiredAccess) const override;

        /// @copydoc IWindowsApiFacade::CreateSvc
        void* CreateSvc(void* serviceHandle, const std::string& serviceName, const std::string& exePath) const override;

        /// @copydoc IWindowsApiFacade::DeleteSvc
        bool DeleteSvc(void* serviceHandle) const override;
    };
} // namespace windows_api_facade
