#include <filesystem.hpp>
#include <filesystem>


namespace filesystem
{
    bool FileSystem::exists(const std::filesystem::path& path) const
    {
        return std::filesystem::exists(path);
    }

    bool FileSystem::is_directory(const std::filesystem::path& path) const
    {
        return std::filesystem::is_directory(path);
    }

    std::uintmax_t FileSystem::remove_all(const std::filesystem::path& path) const
    {
        return std::filesystem::remove_all(path);
    }

    std::filesystem::path FileSystem::temp_directory_path() const
    {
        return std::filesystem::temp_directory_path();
    }

    bool FileSystem::create_directories(const std::filesystem::path& path) const
    {
        return std::filesystem::create_directories(path);
    }

    void FileSystem::rename(const std::filesystem::path& from, const std::filesystem::path& to) const
    {
        std::filesystem::rename(from, to);
    }

    bool FileSystem::remove(const std::filesystem::path& path) const
    {
        return std::filesystem::remove(path);
    }
}
