#include "file_reader.hpp"
#include <logcollector.hpp>
#include <logger.hpp>

#include <algorithm>
#include <string>

constexpr auto FILE_WAIT = std::chrono::milliseconds(500);
constexpr auto RELOAD_INTERVAL = std::chrono::seconds(60);

// UNIX
#include <glob.h>

using namespace logcollector;
using namespace std;

constexpr auto BUFFER_SIZE = 4096;

FileReader::FileReader(Logcollector& logcollector, string globexp) :
    IReader(logcollector),
    m_fileGlob(std::move(globexp)),
    m_localfiles() { }

Awaitable FileReader::Run() {
    while (true) {
        Reload([&](Localfile & lf) {
            lf.SeekEnd();
            m_logcollector.EnqueueTask(ReadLocalfile(&lf));
        });

        co_await m_logcollector.Wait(RELOAD_INTERVAL);
    }
}

void FileReader::Reload(const function<void (Localfile &)> & callback) {
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

Awaitable FileReader::ReadLocalfile(Localfile* lf) {
    while (true) {
        auto log = lf->NextLog();

        while (!log.empty()) {
            m_logcollector.SendMessage(lf->Filename(), log);
            log = lf->NextLog();
        }

        co_await m_logcollector.Wait(FILE_WAIT);
    }
}

void FileReader::AddLocalfiles(const list<string>& paths, const function<void (Localfile &)> & callback) {
    for (auto & path : paths) {
        if (none_of(m_localfiles.begin(), m_localfiles.end(), [&path](Localfile & lf) { return lf.Filename() == path; })) {
            m_localfiles.emplace_back(path);
            LogInfo("Reading log file: {}", m_localfiles.back().Filename());
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

OpenError::OpenError(const string& filename) :
    m_what(string("Cannot open file: ") + filename) { }

const char * OpenError::what() const noexcept {
    return m_what.c_str();
}
