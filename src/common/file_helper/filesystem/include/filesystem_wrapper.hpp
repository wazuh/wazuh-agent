#pragma once

#include <ifilesystem_wrapper.hpp>

namespace filesystem
{
    /// @brief A wrapper class for file system operations, implementing the IFileSystem interface.
    ///
    /// This class provides methods for file system operations such as checking if a file exists,
    /// removing directories, creating directories, and renaming files, among others. It is designed
    /// to be used as a concrete implementation of the IFileSystem interface, encapsulating the actual
    /// file system operations.
    class FileSystemWrapper : public IFileSystemWrapper
    {
    public:
        /// @copydoc IFileSystemWrapper::exists
        bool exists(const std::filesystem::path& path) const override;

        /// @copydoc IFileSystemWrapper::is_directory
        bool is_directory(const std::filesystem::path& path) const override;

        /// @copydoc IFileSystemWrapper::is_regular_file
        bool is_regular_file(const std::filesystem::path& path) const override;

        /// @copydoc IFileSystemWrapper::is_socket
        bool is_socket(const std::filesystem::path& path) const override;

        /// @copydoc IFileSystemWrapper::remove_all
        std::uintmax_t remove_all(const std::filesystem::path& path) const override;

        /// @copydoc IFileSystemWrapper::temp_directory_path
        std::filesystem::path temp_directory_path() const override;

        /// @copydoc IFileSystemWrapper::create_directories
        bool create_directories(const std::filesystem::path& path) const override;

        /// @copydoc IFileSystemWrapper::list_directory
        std::vector<std::filesystem::path> list_directory(const std::filesystem::path& path) const override;

        /// @copydoc IFileSystemWrapper::rename
        void rename(const std::filesystem::path& from, const std::filesystem::path& to) const override;

        /// @copydoc IFileSystemWrapper::remove
        bool remove(const std::filesystem::path& path) const override;
    };
} // namespace filesystem_wrapper
