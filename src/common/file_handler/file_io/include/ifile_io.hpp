#pragma once

#include <filesystem>
#include <fstream>
#include <functional>
#include <string>

/// @brief Interface for file IO operations.
///
/// This interface defines a set of file IO operations such as reading lines from a file,
/// getting the content of a file, and getting the binary content of a file.
class IFileIO
{
public:
    /// @brief Virtual destructor for IFileIO.
    ///
    /// Ensures that any derived classes with their own resources are correctly cleaned up.
    virtual ~IFileIO() = default;

    virtual std::ifstream create_ifstream(const std::string& filePath,
                                          std::ios_base::openmode mode = std::ios_base::in) const;

    virtual std::streambuf* get_rdbuf(const std::ifstream& file) const = 0;

    virtual bool is_open(const std::ifstream& file) const = 0;

    virtual bool get_line(std::istream& file, std::string& line) const = 0;

    /// @brief Reads lines from a file and calls a callback for each line.
    /// @param filePath The path to the file to read.
    /// @param callback The callback function to call for each line.
    virtual void readLineByLine(const std::filesystem::path& filePath,
                                const std::function<bool(const std::string&)>& callback) const = 0;

    /// @brief Gets the content of a file as a string.
    /// @param filePath The path to the file to read.
    /// @return The content of the file as a string.
    virtual std::string getFileContent(const std::string& filePath) const = 0;

    /// @brief Gets the binary content of a file as a vector of bytes.
    /// @param filePath The path to the file to read.
    /// @return The binary content of the file as a vector of bytes.
    virtual std::vector<char> getBinaryContent(const std::string& filePath) const = 0;
};
