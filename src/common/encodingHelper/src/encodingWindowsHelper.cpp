#include "encodingWindowsHelper.hpp"

// clang-format off
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <memory>
// clang-format on

namespace Utils
{
    std::string wstringToStringUTF8(const std::wstring& inputArgument)
    {
        std::string retVal;

        if (!inputArgument.empty())
        {
            const auto inputArgumentSize {static_cast<int>(inputArgument.size())};
            const auto sizeNeeded {
                WideCharToMultiByte(CP_UTF8, 0, inputArgument.data(), inputArgumentSize, nullptr, 0, nullptr, nullptr)};
            const auto buffer {std::make_unique<char[]>(sizeNeeded)};

            if (WideCharToMultiByte(
                    CP_UTF8, 0, inputArgument.data(), inputArgumentSize, buffer.get(), sizeNeeded, nullptr, nullptr) >
                0)
            {
                retVal.assign(buffer.get(), sizeNeeded);
            }
        }

        return retVal;
    }

    std::wstring stringToWStringAnsi(const std::string& inputArgument)
    {
        std::wstring retVal;

        if (!inputArgument.empty())
        {
            const auto inputArgumentSize {static_cast<int>(inputArgument.size())};
            const auto sizeNeeded {MultiByteToWideChar(CP_ACP, 0, inputArgument.data(), inputArgumentSize, nullptr, 0)};
            const auto buffer {std::make_unique<wchar_t[]>(sizeNeeded)};

            if (MultiByteToWideChar(CP_ACP, 0, inputArgument.data(), inputArgumentSize, buffer.get(), sizeNeeded) > 0)
            {
                retVal.assign(buffer.get(), sizeNeeded);
            }
        }

        return retVal;
    }

    std::string stringAnsiToStringUTF8(const std::string& inputArgument)
    {
        return wstringToStringUTF8(stringToWStringAnsi(inputArgument));
    }
} // namespace Utils
