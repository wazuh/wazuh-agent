#pragma once

#include <ifilesystem_utils.hpp>
#include <ifilesystem_wrapper.hpp>

#include <memory>

namespace filesystem
{
    /// @brief A wrapper class for file system operations, implementing the IFileSystem interface.
    ///
    /// This class provides methods for file system operations such as checking if a file exists,
    /// removing directories, creating directories, and renaming files, among others. It is designed
    /// to be used as a concrete implementation of the IFileSystem interface, encapsulating the actual
    /// file system operations.
    class FileSystemUtils : public IFileSystemUtils
    {
    public:
        /// @brief Constructor for the FileSystemUtils class.
        FileSystemUtils(std::shared_ptr<IFileSystemWrapper> fsWrapper = nullptr);

        /// @copydoc IFileSystem::expand_absolute_path
        void expand_absolute_path(const std::string& path, std::deque<std::string>& output) const override;

    private:
        std::shared_ptr<IFileSystemWrapper> m_fsWrapper;
    };
} // namespace filesystem_wrapper
