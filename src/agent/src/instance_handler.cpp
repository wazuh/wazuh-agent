#include <instance_handler.hpp>

namespace instance_handler
{
    InstanceHandler::InstanceHandler(std::string lockFilePath)
        : m_lockFilePath(std::move(lockFilePath))
        , m_errno(0)
        , m_lockAcquired(getInstanceLock())
    {
    }

    bool InstanceHandler::isLockAcquired() const
    {
        return m_lockAcquired;
    }

    int InstanceHandler::getErrno() const
    {
        return m_errno;
    }
} // namespace instance_handler
