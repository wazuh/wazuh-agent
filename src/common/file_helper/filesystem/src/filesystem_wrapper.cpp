#include <filesystem>
#include <filesystem_wrapper.hpp>

#include "../../../globHelper/include/globHelper.h" //fix

#include <array>

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

    bool FileSystemWrapper::is_regular_file(const std::filesystem::path& path) const
    {
        return std::filesystem::is_regular_file(path);
    }

    bool FileSystemWrapper::is_socket(const std::filesystem::path& path) const
    {
        return std::filesystem::is_socket(path);
    }

    std::uintmax_t FileSystemWrapper::remove_all(const std::filesystem::path& path) const
    {
        return std::filesystem::remove_all(path);
    }

    std::filesystem::path FileSystemWrapper::temp_directory_path() const
    {
        return std::filesystem::temp_directory_path();
    }

    bool FileSystemWrapper::create_directories(const std::filesystem::path& path) const
    {
        return std::filesystem::create_directories(path);
    }

    std::vector<std::filesystem::path> FileSystemWrapper::list_directory(const std::filesystem::path& path) const
    {
        std::vector<std::filesystem::path> result;
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            result.push_back(entry.path());
        }
        return result;
    }

    void FileSystemWrapper::rename(const std::filesystem::path& from, const std::filesystem::path& to) const
    {
        std::filesystem::rename(from, to);
    }

    bool FileSystemWrapper::remove(const std::filesystem::path& path) const
    {
        return std::filesystem::remove(path);
    }

    // NOLINTNEXTLINE(misc-no-recursion)
    void FileSystemWrapper::expand_absolute_path(const std::string& path, std::deque<std::string>& output) const
    {
        // Find the first * or ? from path.
        const std::array<char, 2> wildcards {'*', '?'};
        size_t wildcardPos = std::string::npos;

        for (const auto& wildcard : wildcards)
        {
            // Find the first wildcard.
            const auto pos = path.find_first_of(wildcard);

            // If the wildcard is found and it is before the current wildcard, then update the wildcard position.
            if (pos != std::string::npos && (wildcardPos == std::string::npos || pos < wildcardPos))
            {
                wildcardPos = pos;
            }
        }

        if (wildcardPos != std::string::npos)
        {
            const auto parentDirectoryPos {path.find_last_of(std::filesystem::path::preferred_separator, wildcardPos)};

            // The parent directory is the part of the path before the first wildcard.
            // If the wildcard is the first character, then the parent directory is the root directory.
            const auto nextDirectoryPos {
                wildcardPos == 0 ? 0 : path.find_first_of(std::filesystem::path::preferred_separator, wildcardPos)};

            if (parentDirectoryPos == std::string::npos)
            {
                throw std::runtime_error {"Invalid path: " + path};
            }

            // The base directory is the part of the path before the first wildcard.
            // If there is no wildcard, then the base directory is the whole path.
            // If the wildcard is the first character, then the base directory is the root directory.
            std::string baseDir;

            if (wildcardPos == 0)
            {
                baseDir = "";
            }
            else
            {
                baseDir = path.substr(0, parentDirectoryPos);
            }

            // The pattern is the part of the path after the first wildcard.
            // If the wildcard is the last character, then the pattern is the rest of the string.
            // If the wildcard is the first character, then the pattern is the rest of the string, minus the next '\'.
            // If there is no next '\', then the pattern is the rest of the string.
            const auto pattern {
                path.substr(parentDirectoryPos == 0 ? 0 : parentDirectoryPos + 1,
                            nextDirectoryPos == std::string::npos
                                ? std::string::npos
                                : nextDirectoryPos - (parentDirectoryPos == 0 ? 0 : parentDirectoryPos + 1))};

            if (exists(baseDir))
            {
                for (const auto& entry : list_directory(baseDir))
                {
                    const auto entryName {entry.filename().string()};

                    if (Utils::patternMatch(entryName, pattern))
                    {
                        std::string nextPath;
                        nextPath += baseDir;
                        nextPath += std::filesystem::path::preferred_separator;
                        nextPath += entryName;
                        nextPath += nextDirectoryPos == std::string::npos ? "" : path.substr(nextDirectoryPos);

                        expand_absolute_path(nextPath, output);
                    }
                }
            }
        }
        else
        {
            output.push_back(path);
        }
    }
} // namespace filesystem_wrapper
