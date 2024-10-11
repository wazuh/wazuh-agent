#include "file_reader.hpp"
#include <logcollector.hpp>
#include <logger.hpp>

#include <string>
#include <vector>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

// POSIX only
#include <fcntl.h>
#include <cerrno>
#include <cstring>

using namespace logcollector;
using namespace std;
using namespace boost::asio;

constexpr auto BUFFER_SIZE = 4096;
constexpr auto FILE_WAIT = std::chrono::milliseconds(500);

Awaitable FileReader::run() {
    auto & logcollector = Logcollector::Instance();

    m_localfiles.emplace_back("/var/log/syslog");
    m_localfiles.emplace_back("/root/test/test.log");

    for (auto & lf : m_localfiles) {
        logcollector.EnqueueTask(lf.run());
    }

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
    auto executor = co_await this_coro::executor;
    auto fd = open(m_filePath.c_str(), O_RDONLY | O_CLOEXEC, 0);

    if (fd == -1) {
        LogWarn("Cannot open file '{}': {}", m_filePath, strerror(errno));
        co_return;
    }

    lseek(fd, 0, SEEK_END);
    auto file = posix::stream_descriptor(executor, fd);
    auto chunk = vector<char>(BUFFER_SIZE);
    string data;

    while (true) {
        size_t n = 0;

        try {
            n = co_await file.async_read_some(buffer(chunk), use_awaitable);
        } catch (boost::system::system_error &) {
            n = 0;
        }

        if (n == 0) {
            co_await steady_timer(executor, FILE_WAIT).async_wait(use_awaitable);
            continue;
        }

        data.append(chunk.data(), n);
        std::size_t newline_pos = 0;

        while ((newline_pos = data.find('\n')) != string::npos) {
            auto line = data.substr(0, newline_pos);
            logcollector.SendMessage(m_filePath, line);
            data.erase(0, newline_pos + 1);
        }
    }

    co_return;
}
