#pragma once

#include <string>
#include <vector>

namespace os_utils
{
    class IOsUtils
    {
    public:
        /// @brief Virtual destructor for IOsUtils.
        /// @note Ensures that any derived classes with their own resources are correctly cleaned up.
        virtual ~IOsUtils() = default;

        /// @brief Get the list of all running processes.
        /// @return A vector of strings containing the names of running processes. Empty if none or error found.
        virtual std::vector<std::string> GetRunningProcesses() = 0;
    };
} // namespace os_utils
