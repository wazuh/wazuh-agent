#include "file_reader.hpp"
#include <logcollector.hpp>
#include <logger.hpp>

#include <string>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

// POSIX only
#include <fcntl.h>
#include <cerrno>
#include <cstring>

using namespace logcollector;
using namespace std;
using boost::asio::posix::stream_descriptor;
using boost::asio::mutable_buffer;
using boost::asio::use_awaitable;

constexpr auto BUFFER_SIZE = 4096;

Awaitable FileReader::run() {
    auto & logcollector = Logcollector::Instance();

    string path = "/var/log/syslog";
    auto lf = Localfile(path);
    logcollector.EnqueueTask(lf.run());

    // TODO: Resolve wildcards
    // TODO: Check for file rotation

    co_return;
}

Localfile::Localfile(string filePath) :
    m_filePath(std::move(filePath)),
    m_offset(0),
    m_inode(0) { }

Awaitable Localfile::run() {

    // TODO: Save/load file state

    auto & logcollector = Logcollector::Instance();
    logcollector.SendMessage("test", "Hello World (from Localfile)");

    auto fd = open(m_filePath.c_str(), O_RDONLY | O_CLOEXEC, 0);

    if (fd == -1) {
        LogWarn("Cannot open file '{}': {}", m_filePath, strerror(errno));
        co_return;
    }

    auto file = stream_descriptor(logcollector.GetExecutor(), fd);
    string data;

    while (true) {
        char buffer[BUFFER_SIZE];
        std::size_t n = co_await file.async_read_some(boost::asio::buffer(buffer), use_awaitable);

        if (n == 0) {
            continue;
        }

        data.append(buffer, n);
        std::size_t newline_pos = 0;

        while ((newline_pos = data.find('\n')) != std::string::npos) {
            auto line = data.substr(0, newline_pos);
            logcollector.SendMessage(m_filePath, line);
            data.erase(0, newline_pos + 1);
        }
    }

    co_return;
}
