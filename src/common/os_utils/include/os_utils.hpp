#pragma once

#include <ios_utils.hpp>

#include <string>
#include <vector>

namespace os_utils
{
    class OsUtils : public IOsUtils
    {
    public:
        /// @brief Destructor for OsUtils.
        ~OsUtils() = default;

        /// @copydoc IOsUtils::GetRunningProcesses
        std::vector<std::string> GetRunningProcesses() override;
    };
} // namespace os_utils
