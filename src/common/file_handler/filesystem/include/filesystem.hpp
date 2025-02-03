#pragma once

#include <ifilesystem.hpp>
#include <filesystem>

namespace filesystem
{
    /// @brief A wrapper class for file system operations, implementing the IFileSystem interface.
    ///
    /// This class provides methods for file system operations such as checking if a file exists,
    /// removing directories, creating directories, and renaming files, among others. It is designed
    /// to be used as a concrete implementation of the IFileSystem interface, encapsulating the actual
    /// file system operations.
    class FileSystem : public IFileSystem {
    public:
        /// @brief Checks if the specified path exists in the file system.
        /// @param path The path to check.
        /// @return Returns true if the path exists, otherwise false.
        bool exists(const std::filesystem::path& path) const override;

        /// @brief Checks if the specified path is a directory.
        /// @param path The path to check.
        /// @return Returns true if the path is a directory, otherwise false.
        bool is_directory(const std::filesystem::path& path) const override;

        /// @brief Checks if the specified path is a regular file.
        /// @param path The path to check.
        /// @return Returns true if the path is a regular file, otherwise false.
        bool is_regular_file(const std::filesystem::path& path) const override;

        /// @brief Removes all files and subdirectories in the specified directory.
        /// @param path The directory path to remove.
        /// @return Returns the number of files and directories removed.
        std::uintmax_t remove_all(const std::filesystem::path& path) const override;

        /// @brief Retrieves the system's temporary directory path.
        /// @return Returns the path of the system's temporary directory.
        std::filesystem::path temp_directory_path() const override;

        /// @brief Creates a new directory at the specified path.
        /// @param path The path of the directory to create.
        /// @return Returns true if the directory was successfully created, otherwise false.
        bool create_directories(const std::filesystem::path& path) const override;

        /// @brief Returns the iterator to the first element of a directory
        /// @param path Path to the directory
        /// @return The iterator to the first element of a directory
        std::filesystem::directory_iterator directory_iterator(const std::filesystem::path& path) const override;

        /// @brief Renames a file or directory from the 'from' path to the 'to' path.
        /// @param from The current path of the file or directory.
        /// @param to The new path for the file or directory.
        void rename(const std::filesystem::path& from, const std::filesystem::path& to) const override;

        /// @brief Removes the specified file or directory.
        /// @param path The file or directory to remove.
        /// @return Returns true if the file or directory was successfully removed, otherwise false.
        bool remove(const std::filesystem::path& path) const override;

        /// @brief Expands the absolute path of a file or directory.
        /// @param path The path to expand.
        /// @param output The deque to store the expanded path.
        void expand_absolute_path(const std::string& path, std::deque<std::string>& output) const override;
    };
}
