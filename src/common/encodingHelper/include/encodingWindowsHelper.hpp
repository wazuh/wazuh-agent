#pragma once

#include <string>

namespace Utils
{
    std::string wstringToStringUTF8(const std::wstring& inputArgument);

    std::wstring stringToWStringAnsi(const std::string& inputArgument);

    std::string stringAnsiToStringUTF8(const std::string& inputArgument);
} // namespace Utils
