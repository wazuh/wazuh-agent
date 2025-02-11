#pragma once

#include "ifile_io.hpp"

#include <filesystem>
#include <functional>
#include <string>

namespace file_io
{
    /// @copydoc IFileIO
    class FileIO : public IFileIO
    {
    public:
        std::ifstream create_ifstream(const std::string& filePath,
                                      std::ios_base::openmode mode = std::ios_base::in) const override;

        std::streambuf* get_rdbuf(const std::ifstream& file) const override;

        bool is_open(const std::ifstream& file) const override;

        bool get_line(std::istream& file, std::string& line) const override;

        /// @copydoc IFileIO::readLineByLine
        void readLineByLine(const std::filesystem::path& filePath,
                            const std::function<bool(const std::string&)>& callback) const override;

        /// @copydoc IFileIO::getFileContent
        std::string getFileContent(const std::string& filePath) const override;

        /// @copydoc IFileIO::getBinaryContent
        std::vector<char> getBinaryContent(const std::string& filePath) const override;
    };
} // namespace file_io
