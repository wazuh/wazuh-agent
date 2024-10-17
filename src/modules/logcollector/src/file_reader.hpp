#pragma once

#include <list>
#include <fstream>
#include <exception>

#include <logcollector.hpp>
#include "reader.hpp"

namespace logcollector {

constexpr auto DEFAULT_FILE_WAIT = 500;
constexpr auto DEFAULT_RELOAD_INTERVAL = 60;

class Localfile {
public:
    Localfile(std::string filename);
    Localfile(std::shared_ptr<std::istream> stream);
    std::string NextLog();
    void SeekEnd();
    bool Rotated();
    void Reopen();
    inline const std::string& Filename() const { return m_filename; }

private:
    std::string m_filename;
    std::shared_ptr<std::istream> m_stream;
    std::streampos m_pos;
};

class FileReader : public IReader {
public:
    FileReader(Logcollector& logcollector, std::string pattern, long fileWait = DEFAULT_FILE_WAIT, long reloadInterval = DEFAULT_RELOAD_INTERVAL);
    Awaitable Run();
    void Reload(const std::function<void (Localfile &)> & callback);
    Awaitable ReadLocalfile(Localfile* lf);

private:
    void AddLocalfiles(const std::list<std::string>& paths, const std::function<void (Localfile &)> & callback);
    void RemoveLocalfile(const std::string& filename);

    std::string m_filePattern;
    std::list<Localfile> m_localfiles;
    long m_fileWaitMs;
    long m_reloadIntervalSec;
};

class OpenError : public std::exception {
public:
    OpenError(const std::string& filename);
    const char * what() const noexcept override;

private:
    std::string m_what;
};

}
