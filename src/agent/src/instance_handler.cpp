#include <instance_handler.hpp>

#include <filesystem_wrapper.hpp>

namespace instance_handler
{
    InstanceHandler::InstanceHandler(std::string lockFilePath, std::shared_ptr<IFileSystemWrapper> fileSystemWrapper)
        : m_lockFilePath(std::move(lockFilePath))
        , m_errno(0)
        , m_fileSystemWrapper(fileSystemWrapper ? fileSystemWrapper
                                                : std::make_shared<file_system::FileSystemWrapper>())
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
