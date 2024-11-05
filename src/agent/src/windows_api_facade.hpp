#pragma once

#include <iwindows_api_facade.hpp>

namespace windows_api_facade
{
    /// @brief Class that provides an interface to Windows API for service management.
    class WindowsApiFacade : public IWindowsApiFacade
    {
    public:
        /// @brief Opens a handle to the service control manager.
        /// @param desiredAccess The access permission to the service control manager.
        /// @return A handle to the service control manager, or NULL if the function fails.
        void* OpenSCM(unsigned int desiredAccess) const override;

        /// @brief Opens a handle to the specified service.
        /// @param serviceHandle A handle to the service control manager.
        /// @param serviceName The name of the service to open.
        /// @param desiredAccess The access permission to the service.
        /// @return A handle to the service, or NULL if the function fails.
        void* OpenSvc(void* serviceHandle, const std::string& serviceName, unsigned int desiredAccess) const override;

        /// @brief Creates a new service.
        /// @param serviceHandle A handle to the service control manager.
        /// @param serviceName The name of the new service.
        /// @param exePath The path to the executable file for the new service.
        /// @return A handle to the new service, or NULL if the function fails.
        void* CreateSvc(void* serviceHandle, const std::string& serviceName, const std::string& exePath) const override;

        /// @brief Deletes the service.
        /// @param serviceHandle A handle to the service to delete.
        /// @return TRUE if the function succeeds, FALSE if it fails.
        bool DeleteSvc(void* serviceHandle) const override;
    };
} // namespace windows_api_facade
