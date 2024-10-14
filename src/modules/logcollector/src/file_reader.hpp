#pragma once

#include <vector>
#include <fstream>
#include <exception>

#include "reader.hpp"

namespace logcollector {

class Localfile {
public:
    Localfile(std::string filename);
    Localfile(std::shared_ptr<std::istream> stream);
    //Awaitable run();
    std::string NextLog();

private:
    // Awaitable Follow();
    // void SplitData(std::string & accumulator);

    std::string m_filename;
    std::shared_ptr<std::istream> m_stream;
    std::streampos m_pos;
};

class FileReader : public IReader {
public:
    Awaitable run();

private:
    std::string m_fileGlob;
    int m_ageThreshold;
    std::string delimRegex;
    bool m_useBookmark;
    std::vector<Localfile> m_localfiles;
};

class OpenError : public std::exception {
public:
    OpenError(const std::string& filename);
    const char * what() const noexcept override;

private:
    std::string m_what;
};

}
