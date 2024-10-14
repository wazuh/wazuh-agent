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
// constexpr auto FILE_WAIT = std::chrono::milliseconds(500);

// Awaitable FileReader::run() {
//     // auto & logcollector = Logcollector::Instance();

//     m_localfiles.emplace_back("/var/log/syslog");
//     m_localfiles.emplace_back("/root/test/test.log");

//     // for (auto & lf : m_localfiles) {
//     //     // logcollector.EnqueueTask(lf.run());
//     // }

//     // TODO: Resolve wildcards
//     // TODO: Check for file rotation
// }

Localfile::Localfile(string filename) :
    m_filename(std::move(filename)),
    m_stream(make_shared<ifstream>(m_filename)),
    m_pos() {
        if (m_stream->fail()) {
            throw OpenError(m_filename);
        }
    }

Localfile::Localfile(shared_ptr<istream> stream) :
    m_filename(),
    m_stream(std::move(stream)) { }

string Localfile::NextLog() {
    auto buffer = vector<char>(BUFFER_SIZE);

    if (m_stream->getline(buffer.data(), BUFFER_SIZE).good()) {
        m_pos = m_stream->tellg();
        return { buffer.data(), static_cast<size_t>(m_stream->gcount()) - 1 };
    } else {
        m_stream->seekg(m_pos);
        return { };
    }
}

// Localfile::Localfile(std::string filePath, boost::asio::any_io_executor & executor) :
//     m_filePath(std::move(filePath)),
//     m_offset(0),
//     m_inode(0),
//     m_file(executor) { }

// Awaitable Localfile::run() {
//     // TODO: Save/load file state

//     m_file = make_unique<FileWrapper>(m_filePath);

//     if (!m_file->good()) {
//         LogWarn("Cannot open file '{}': {}", m_filePath, strerror(errno));
//         co_return;
//     }

//     lseek(fd, 0, SEEK_END);
//     m_file.assign(fd);

//     co_await Follow();
// }

// Awaitable Localfile::Follow() {
//     auto executor = co_await this_coro::executor;
//     auto chunk = vector<char>(BUFFER_SIZE);
//     string accumulator;

//     while (true) {
//         bool n;

//         try {
//             n = m_file.getline(chunk);
//         } catch (boost::system::system_error &) {
//             n = 0;
//         }

//         if (!n) {
//             co_await steady_timer(executor, FILE_WAIT).async_wait(use_awaitable);
//             continue;
//         }

//         accumulator.append(chunk.data(), n);
//         SplitData(accumulator);
//     }
// }

// void Localfile::SplitData(string & accumulator) {
//     auto & logcollector = Logcollector::Instance();
//     std::size_t newline_pos = 0;

//     while ((newline_pos = accumulator.find('\n')) != string::npos) {
//         auto line = accumulator.substr(0, newline_pos);
//         logcollector.SendMessage(m_filePath, line);
//         accumulator.erase(0, newline_pos + 1);
//     }
// }

OpenError::OpenError(const string& filename) :
    m_what(string("Cannot open file: ") + filename) { }

const char * OpenError::what() const noexcept {
    return m_what.c_str();
}
