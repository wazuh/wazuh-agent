#include <file_io.hpp>

#include <fstream>

namespace file_io
{
    void FileIO::readLineByLine(const std::filesystem::path& filePath,
        const std::function<bool(const std::string&)>& callback)
    {
        std::ifstream file(filePath);

        if (!file.is_open())
        {
            throw std::runtime_error("Could not open file");
        }

        std::string line;

        while (std::getline(file, line))
        {
            if (!callback(line))
            {
                break;
            }
        }
    }
};
