#include <gmock/gmock.h>

#include <ifilesystem_wrapper.hpp>

class MockFileSystemWrapper : public IFileSystemWrapper
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
};
