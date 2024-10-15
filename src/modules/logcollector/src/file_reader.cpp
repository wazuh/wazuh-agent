#include "file_reader.hpp"
#include <logcollector.hpp>
#include <logger.hpp>

#include <algorithm>
#include <string>

constexpr auto FILE_WAIT = std::chrono::milliseconds(500);

// UNIX
#include <glob.h>

using namespace logcollector;
using namespace std;

constexpr auto BUFFER_SIZE = 4096;

FileReader::FileReader(Logcollector& logcollector, string globexp) :
    m_logcollector(logcollector),
    m_fileGlob(std::move(globexp)),
    m_localfiles() { }

Awaitable FileReader::Run(Logcollector& logcollector) {
    Reload([&](Localfile & lf) {
        lf.SeekEnd();
        logcollector.EnqueueTask(ReadLocalfile(lf));
    });

    // TODO: Check for file rotation

    co_return;
}

void FileReader::Reload(function<void (Localfile &)> callback) {
    glob_t globResult;

    int ret = glob(m_fileGlob.c_str(), 0, nullptr, &globResult);

    if (ret != 0) {
        if (ret == GLOB_NOMATCH) {
            LogTrace("No matches found for pattern: {}", m_fileGlob);
        } else {
            LogWarn("Cannot use glob with pattern: {}", m_fileGlob);
        }

        globfree(&globResult);
        return;
    }

    list<string> localfiles;
    auto paths = std::span<char *>(globResult.gl_pathv, globResult.gl_pathc);

    for (auto& path : paths) {
        localfiles.emplace_back(path);
    }

    AddLocalfiles(localfiles, callback);
    globfree(&globResult);
}

Awaitable FileReader::ReadLocalfile(Localfile& lf) {
    while (true) {
        auto log = lf.NextLog();

        while (!log.empty()) {
            m_logcollector.SendMessage(lf.Filename(), log);
            log = lf.NextLog();
        }

        co_await m_logcollector.Wait(FILE_WAIT);
    }

    co_return;
}

void FileReader::AddLocalfiles(const list<string>& paths, function<void (Localfile &)> callback) {
    for (auto & path : paths) {
        if (none_of(m_localfiles.begin(), m_localfiles.end(), [&path](Localfile & lf) { return lf.Filename() == path; })) {
            m_localfiles.emplace_back(path);
            callback(m_localfiles.back());
        }
    }
}

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
        m_stream->clear();
        return { };
    }
}

void Localfile::SeekEnd() {
    m_stream->seekg(0, ios::end);
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
