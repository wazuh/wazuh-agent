#include <file_io_wrapper.hpp>

#include <fstream>
#include <sstream>

namespace file_io
{
    std::ifstream FileIOWrapper::create_ifstream(const std::string& filePath, std::ios_base::openmode mode) const
    {
        return std::ifstream(filePath, mode);
    }

    std::streambuf* FileIOWrapper::get_rdbuf(const std::ifstream& file) const
    {
        return file.rdbuf();
    }

    bool FileIOWrapper::is_open(const std::ifstream& file) const
    {
        return file.is_open();
    }

    bool FileIOWrapper::get_line(std::istream& file, std::string& line) const
    {
        return static_cast<bool>(std::getline(file, line));
    }
}; // namespace file_io
