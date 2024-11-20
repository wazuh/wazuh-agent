#pragma once

#include <list>
#include <fstream>
#include <exception>
#include <ctime>

#include <logcollector.hpp>
#include "reader.hpp"
#include <config.h>

namespace logcollector {

/// @brief Local file class
///
/// This class represents an individual local file that can be read by
/// Logcollector.
class Localfile {
public:
    /// @brief Constructor
    /// @param filename File name
    Localfile(std::string filename);

    /// @brief Constructor
    /// @param stream Shared pointer to an input stream
    Localfile(std::shared_ptr<std::istream> stream);

    /// @brief Gets the next log from the file
    /// @return A log, or an empty string if the end of the file has been reached
    std::string NextLog();

    /// @brief Seeks to the end of the file
    void SeekEnd();

    /// @brief Checks if the file has been rotated
    ///
    /// This method checks if the file has been rotated by comparing the current
    /// size of the file with the reading position. If the file size is lower
    /// than the reading position, the file has been rotated.
    ///
    /// @return True if the file has been rotated, false otherwise
    bool Rotated();

    /// @brief Reopens the file
    void Reopen();

    /// @brief Gets the file name
    /// @return File name
    inline const std::string& Filename() const { return m_filename; }

private:
    /// @brief File name
    std::string m_filename;

    /// @brief Shared pointer to the input stream
    std::shared_ptr<std::istream> m_stream;

    /// @brief Current position in the file
    std::streampos m_pos;
};

/// @brief File reader class
///
/// This class represents each file block in the module. There may exist
/// multiple file readers of each type. The File reader expands wildcards so
/// that one file reader can read multiple files (Localfile).
class FileReader : public IReader {
public:
    /// @brief Constructor for the file reader
    ///
    /// @param logcollector Log collector instance
    /// @param pattern File pattern
    /// @param fileWait File wait time in milliseconds
    /// @param reloadInterval Reload interval in milliseconds
    FileReader(Logcollector& logcollector, std::string pattern, std::time_t fileWait, std::time_t reloadInterval);

    /// @brief Runs the file reader
    /// @return Awaitable result
    Awaitable Run();

    /// @brief Reloads the file list
    ///
    /// Expands wildcards in the file pattern. Adds the new files to the list,
    /// and calls the callback function.
    ///
    /// @param callback Callback function
    void Reload(const std::function<void (Localfile &)> & callback);

    /// @brief Reads a local file
    /// @param lf Localfile
    /// @return Awaitable result
    Awaitable ReadLocalfile(Localfile* lf);

private:
    /// @brief Adds localfiles to the list
    ///
    /// Merges the new files with the existing files. For each new file, it
    /// calls the callback function.
    ///
    /// @param paths List of file paths
    /// @param callback Callback function
    void AddLocalfiles(const std::list<std::string>& paths, const std::function<void (Localfile &)> & callback);

    /// @brief Removes a local file from the list
    /// @param filename File name
    /// @post The file is destroyed and may not be used anymore
    void RemoveLocalfile(const std::string& filename);

    /// @brief File pattern
    std::string m_filePattern;

    /// @brief List of local files
    std::list<Localfile> m_localfiles;

    /// @brief File reading interval in milliseconds
    std::time_t m_fileWait;

    /// @brief Reload (wildcard expand) interval in milliseconds
    std::time_t m_reloadInterval;

    /// @brief File pattern
    const std::string m_collectorType = "file";
};

/// @brief Open error class
///
/// This class represents an error that occurs when opening a file.
class OpenError : public std::exception {
public:
    /// @brief Constructor
    /// @param filename File name
    OpenError(const std::string& filename);

    /// @brief Gets the error message
    /// @return Error message
    const char * what() const noexcept override;

private:
    /// @brief Error message
    std::string m_what;
};

}
