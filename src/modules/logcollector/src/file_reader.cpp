#include "file_reader.hpp"
#include <logcollector.hpp>
#include <logger.hpp>

#include <string>
#include <vector>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

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
    auto executor = co_await this_coro::executor;

    m_localfiles.emplace_back("/var/log/syslog", executor);
    m_localfiles.emplace_back("/root/test/test.log", executor);

    for (auto & lf : m_localfiles) {
        logcollector.EnqueueTask(lf.run());
    }

    // TODO: Resolve wildcards
    // TODO: Check for file rotation
}

Localfile::Localfile(std::string filePath, boost::asio::any_io_executor executor) :
    m_filePath(std::move(filePath)),
    m_offset(0),
    m_inode(0),
    m_file(executor) { }

Awaitable Localfile::run() {
    // TODO: Save/load file state

    auto fd = open(m_filePath.c_str(), O_RDONLY | O_CLOEXEC, 0);

    if (fd == -1) {
        LogWarn("Cannot open file '{}': {}", m_filePath, strerror(errno));
        co_return;
    }

    lseek(fd, 0, SEEK_END);
    m_file.assign(fd);

    co_await Follow();
}

Awaitable Localfile::Follow() {
    auto executor = co_await this_coro::executor;
    auto chunk = vector<char>(BUFFER_SIZE);
    string accumulator;

    while (true) {
        size_t n = 0;

        try {
            n = co_await m_file.async_read_some(buffer(chunk), use_awaitable);
        } catch (boost::system::system_error &) {
            n = 0;
        }

        if (n == 0) {
            co_await steady_timer(executor, FILE_WAIT).async_wait(use_awaitable);
            continue;
        }

        accumulator.append(chunk.data(), n);
        SplitData(accumulator);
    }
}

void Localfile::SplitData(string & accumulator) {
    auto & logcollector = Logcollector::Instance();
    std::size_t newline_pos = 0;

    while ((newline_pos = accumulator.find('\n')) != string::npos) {
        auto line = accumulator.substr(0, newline_pos);
        logcollector.SendMessage(m_filePath, line);
        accumulator.erase(0, newline_pos + 1);
    }
}
