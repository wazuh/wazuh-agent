#pragma once

#include <string>

namespace Utils
{
    /// @brief Convert a wstring to a string
    /// @param inputArgument wstring to convert
    /// @return string
    std::string wstringToStringUTF8(const std::wstring& inputArgument);

    /// @brief Convert a string to a wstring
    /// @param inputArgument string to convert
    /// @return wstring
    std::wstring stringToWStringAnsi(const std::string& inputArgument);

    /// @brief Convert a string to a UTF8 string
    /// @param inputArgument string to convert
    /// @return UTF8 string
    std::string stringAnsiToStringUTF8(const std::string& inputArgument);
} // namespace Utils
