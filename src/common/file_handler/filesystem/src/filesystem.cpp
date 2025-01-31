#include <filesystem_wrapper.hpp>
#include <filesystem>


namespace filesystem_wrapper
{
    bool FileSystemWrapper::exists(const std::filesystem::path& path) const
    {
        return std::filesystem::exists(path);
    }

    bool FileSystemWrapper::is_directory(const std::filesystem::path& path) const
    {
        return std::filesystem::is_directory(path);
    }

    std::uintmax_t FileSystemWrapper::remove_all(const std::filesystem::path& path)
    {
        return std::filesystem::remove_all(path);
    }

    std::filesystem::path FileSystemWrapper::temp_directory_path() const
    {
        return std::filesystem::temp_directory_path();
    }

    bool FileSystemWrapper::create_directories(const std::filesystem::path& path)
    {
        return std::filesystem::create_directories(path);
    }

    void FileSystemWrapper::rename(const std::filesystem::path& from, const std::filesystem::path& to)
    {
        std::filesystem::rename(from, to);
    }

    bool FileSystemWrapper::remove(const std::filesystem::path& path)
    {
        return std::filesystem::remove(path);
    }
}
