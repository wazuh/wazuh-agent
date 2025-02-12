#include <gmock/gmock.h>

#include <ifilesystem.hpp>

class MockFileSystem : public IFileSystem
{
public:
    MOCK_METHOD(bool, exists, (const std::filesystem::path& path), (const, override));
    MOCK_METHOD(bool, is_directory, (const std::filesystem::path& path), (const, override));
    MOCK_METHOD(bool, is_regular_file, (const std::filesystem::path& path), (const, override));
    MOCK_METHOD(bool, is_socket, (const std::filesystem::path& path), (const, override));
    MOCK_METHOD(std::uintmax_t, remove_all, (const std::filesystem::path& path), (const, override));
    MOCK_METHOD(std::filesystem::path, temp_directory_path, (), (const, override));
    MOCK_METHOD(bool, create_directories, (const std::filesystem::path& path), (const, override));
    MOCK_METHOD(std::vector<std::filesystem::path>,
                list_directory,
                (const std::filesystem::path& path),
                (const, override));
    MOCK_METHOD(void, rename, (const std::filesystem::path& from, const std::filesystem::path& to), (const, override));
    MOCK_METHOD(bool, remove, (const std::filesystem::path& path), (const, override));
    MOCK_METHOD(void,
                expand_absolute_path,
                (const std::string& path, std::deque<std::string>& output),
                (const, override));
};
