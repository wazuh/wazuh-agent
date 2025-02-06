#pragma once

#include <filesystem>
#include <functional>
#include <string>

namespace file_io
{
    /// @copydoc IFileIO
    class FileIO
    {
    public:
        /// @copydoc IFileIO::readLineByLine
        void readLineByLine(const std::filesystem::path& filePath,
                            const std::function<bool(const std::string&)>& callback) const;

        /// @copydoc IFileIO::getFileContent
        std::string getFileContent(const std::string& filePath) const;

        /// @copydoc IFileIO::getBinaryContent
        std::vector<char> getBinaryContent(const std::string& filePath) const;
    };
} // namespace file_io
