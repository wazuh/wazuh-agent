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

    bool FileSystem::is_regular_file(const std::filesystem::path& path) const
    {
        return std::filesystem::is_regular_file(path);
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

    std::filesystem::directory_iterator FileSystem::directory_iterator(const std::filesystem::path& path) const
    {
        return std::filesystem::directory_iterator(path);
    }

    std::filesystem::directory_iterator FileSystem::directory_iterator(const std::filesystem::path& path) const
    {
        return std::filesystem::directory_iterator(path);
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
