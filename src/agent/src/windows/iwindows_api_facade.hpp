#pragma once

#include <string>

namespace windows_api_facade
{
    /// @brief Interface to Windows API for service management.
    class IWindowsApiFacade
    {
    public:
        virtual ~IWindowsApiFacade() = default;

        /// @brief Opens a handle to the service control manager.
        /// @param desiredAccess The access permission to the service control manager.
        /// @return A handle to the service control manager, or NULL if the function fails.
        virtual void* OpenSCM(unsigned int desiredAccess) const = 0;

        /// @brief Opens a handle to the specified service.
        /// @param serviceHandle A handle to the service control manager.
        /// @param serviceName The name of the service to open.
        /// @param desiredAccess The access permission to the service.
        /// @return A handle to the service, or NULL if the function fails.
        virtual void*
        OpenSvc(void* serviceHandle, const std::string& serviceName, unsigned int desiredAccess) const = 0;

        /// @brief Creates a new service.
        /// @param serviceHandle A handle to the service control manager.
        /// @param serviceName The name of the new service.
        /// @param exePath The path to the executable file for the new service.
        /// @return A handle to the new service, or NULL if the function fails.
        virtual void*
        CreateSvc(void* serviceHandle, const std::string& serviceName, const std::string& exePath) const = 0;

        /// @brief Deletes the service.
        /// @param serviceHandle A handle to the service to delete.
        /// @return TRUE if the function succeeds, FALSE if it fails.
        virtual bool DeleteSvc(void* serviceHandle) const = 0;
    };
} // namespace windows_api_facade
