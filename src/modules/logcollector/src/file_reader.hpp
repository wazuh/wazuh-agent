#pragma once

#include <vector>
#include <boost/asio/posix/stream_descriptor.hpp>

#include "reader.hpp"

namespace logcollector {


class Localfile {
public:
    Localfile(std::string filePath, boost::asio::any_io_executor executor);
    Awaitable run();

private:
    Awaitable Follow();
    void SplitData(std::string & accumulator);
    std::string m_filePath;
    unsigned long m_offset;
    unsigned long m_inode;
    boost::asio::posix::stream_descriptor m_file;
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
