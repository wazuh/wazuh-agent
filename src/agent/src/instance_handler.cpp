#include <instance_handler.hpp>

namespace unix_daemon
{
    LockFileHandler::LockFileHandler(std::string lockFilePath)
        : m_lockFilePath(std::move(lockFilePath))
        , m_errno(0)
        , m_lockFileCreated(createLockFile())
    {
    }
} // namespace unix_daemon
