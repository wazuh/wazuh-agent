#include <instance_handler.hpp>

#include <logger.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace
{
    const std::string MUTEX_NAME = "Global\\WazuhAgentMutex";
}

namespace instance_handler
{
    InstanceHandler::~InstanceHandler() = default;

    bool InstanceHandler::getInstanceLock()
    {
        SECURITY_ATTRIBUTES sa;
        ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);

        PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
        InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(pSD, TRUE, NULL, FALSE);

        sa.lpSecurityDescriptor = pSD;
        sa.bInheritHandle = FALSE;

        HANDLE hMutex = CreateMutexA(&sa, FALSE, MUTEX_NAME.c_str());

        LocalFree(pSD);
        if (hMutex == nullptr)
        {
            m_errno = GetLastError();
            LogError("Failed to create mutex. Error: {}", m_errno);
            return false;
        }

        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            m_errno = ERROR_ALREADY_EXISTS;
            LogDebug("Failed to create mutex. Error: {}", m_errno);
            CloseHandle(hMutex);
            return false;
        }
        LogInfo("Mutex created");
        return true;
    }

    InstanceHandler GetInstanceHandler([[maybe_unused]] const std::string& configFilePath,
                                       [[maybe_unused]] std::shared_ptr<IFileSystemWrapper> fileSystemWrapper)
    {
        return {InstanceHandler("")};
    }

    void InstanceHandler::releaseInstanceLock() const {}

    std::string GetAgentStatus(const std::string& configFilePath,
                               [[maybe_unused]] std::shared_ptr<IFileSystemWrapper> fileSystemWrapper)
    {
        InstanceHandler instanceHandler = GetInstanceHandler(configFilePath);

        if (!instanceHandler.isLockAcquired())
        {
            if (instanceHandler.getErrno() == ERROR_ALREADY_EXISTS)
            {
                return "running";
            }
            else
            {
                LPSTR message_buffer = nullptr;
                size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                                 FORMAT_MESSAGE_IGNORE_INSERTS,
                                             nullptr,
                                             instanceHandler.getErrno(),
                                             MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                             (LPSTR)&message_buffer,
                                             0,
                                             nullptr);

                std::string message(message_buffer, size);

                LocalFree(message_buffer);

                return fmt::format("Error: {} ({})", instanceHandler.getErrno(), message.c_str());
            }
        }

        return "stopped";
    }
} // namespace instance_handler
