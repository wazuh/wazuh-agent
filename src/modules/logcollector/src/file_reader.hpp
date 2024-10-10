#pragma once

#include <vector>

#include "reader.hpp"

namespace logcollector {

class Localfile {
public:
    Localfile(std::string filePath);
    Awaitable run();

private:
    std::string m_filePath;
    unsigned long m_offset;
    unsigned long m_inode;
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

}
