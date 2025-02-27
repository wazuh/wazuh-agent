#pragma once

#include <ifile_io_wrapper.hpp>

//#include <filesystem>
//#include <functional>
#include <string>

namespace file_io
{
    /// @copydoc IFileIOWrapper
    class FileIOWrapper : public IFileIOWrapper
    {
    public:
        /// @copydoc IFileIO::get_line
        bool get_line(std::istream& file, std::string& line) const override;

        /// @copydoc IFileIO::create_ifstream
        std::ifstream create_ifstream(const std::string& filePath,
                                      std::ios_base::openmode mode = std::ios_base::in) const override;

        /// @copydoc IFileIO::get_rdbuf
        std::streambuf* get_rdbuf(const std::ifstream& file) const override;

        /// @copydoc IFileIO::is_open
        bool is_open(const std::ifstream& file) const override;
    };
} // namespace file_io
