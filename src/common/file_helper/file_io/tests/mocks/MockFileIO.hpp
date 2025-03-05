#include <gmock/gmock.h>

#include <ifile_io_wrapper.hpp>

class MockFileIO : public IFileIO
{
public:
    MOCK_METHOD(std::ifstream,
                create_ifstream,
                (const std::string& filePath, std::ios_base::openmode mode),
                (const, override));
    MOCK_METHOD(std::streambuf*, get_rdbuf, (const std::ifstream& file), (const, override));
    MOCK_METHOD(bool, is_open, (const std::ifstream& file), (const, override));
    MOCK_METHOD(bool, get_line, (std::istream & file, std::string& line), (const, override));
    MOCK_METHOD(void,
                readLineByLine,
                (const std::filesystem::path& filePath, const std::function<bool(const std::string&)>& callback),
                (const, override));
    MOCK_METHOD(std::string, getFileContent, (const std::string& filePath), (const, override));
    MOCK_METHOD(std::vector<char>, getBinaryContent, (const std::string& filePath), (const, override));
};
