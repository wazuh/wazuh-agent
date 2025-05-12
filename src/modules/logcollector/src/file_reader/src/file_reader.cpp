#include "file_reader.hpp"

#include <config.h>
#include <logcollector.hpp>
#include <logger.hpp>

#include <algorithm>
#include <filesystem>
#include <string>

using namespace logcollector;

FileReader::FileReader(Logcollector& logcollector,
                       std::string pattern,
                       std::time_t fileWait,
                       std::time_t reloadInterval)
    : IReader(logcollector)
    , m_filePattern(std::move(pattern))
    , m_localfiles()
    , m_fileWait(fileWait)
    , m_reloadInterval(reloadInterval)
{
}

Awaitable FileReader::Run()
{
    while (m_keepRunning.load())
    {
        Reload(
            [&](Localfile& lf)
            {
                lf.SeekEnd();
                m_logcollector.EnqueueTask(ReadLocalfile(&lf));
            });

        co_await m_logcollector.Wait(std::chrono::milliseconds(m_reloadInterval));
    }
}

void FileReader::Stop()
{
    m_keepRunning.store(false);
}

Awaitable FileReader::ReadLocalfile(Localfile* lf)
{
    while (m_keepRunning.load())
    {
        auto log = lf->NextLog();

        while (!log.empty())
        {
            m_logcollector.PushMessage(lf->Filename(), log, m_collectorType);
            log = lf->NextLog();
        }

        try
        {
            if (lf->Rotated())
            {
                LogInfo("File '{}' rotated, reloading", lf->Filename());
                lf->Reopen();
            }
        }
        catch (OpenError&)
        {
            LogInfo("File inaccesible: {}", lf->Filename());
            co_return;
        }

        co_await m_logcollector.Wait(std::chrono::milliseconds(m_fileWait));
    }

    RemoveLocalfile(lf->Filename());
}

void FileReader::AddLocalfiles(const std::list<std::string>& paths, const std::function<void(Localfile&)>& callback)
{
    for (auto& path : paths)
    {
        if (none_of(m_localfiles.begin(), m_localfiles.end(), [&path](Localfile& lf) { return lf.Filename() == path; }))
        {
            m_localfiles.emplace_back(path);
            LogInfo("Reading log file: {}", m_localfiles.back().Filename());
            callback(m_localfiles.back());
        }
    }
}

void FileReader::RemoveLocalfile(const std::string& filename)
{
    m_localfiles.remove_if([&filename](Localfile& lf) { return lf.Filename() == filename; });
}

Localfile::Localfile(std::string filename)
    : m_filename(std::move(filename))
    , m_stream(make_shared<std::ifstream>(m_filename))
    , m_pos()
{
    if (m_stream->fail())
    {
        throw OpenError(m_filename);
    }
}

Localfile::Localfile(std::shared_ptr<std::istream> stream)
    : m_filename()
    , m_stream(std::move(stream))
{
}

std::string Localfile::NextLog()
{
    auto buffer = std::vector<char>(config::logcollector::BUFFER_SIZE);

    if (m_stream->getline(buffer.data(), config::logcollector::BUFFER_SIZE).good())
    {
        m_pos = m_stream->tellg();
        return {buffer.data(), static_cast<size_t>(m_stream->gcount()) - 1};
    }
    else
    {
        m_stream->seekg(m_pos);
        m_stream->clear();
        return {};
    }
}

void Localfile::SeekEnd()
{
    m_stream->seekg(0, std::ios::end);
}

bool Localfile::Rotated()
{
    try
    {
        auto fileSize = std::filesystem::file_size(m_filename);
        auto streamSize = static_cast<uintmax_t>(m_stream->tellg());
        return fileSize < streamSize;
    }
    catch (std::filesystem::filesystem_error&)
    {
        throw OpenError(m_filename);
    }
}

void Localfile::Reopen()
{
    m_stream = std::make_shared<std::ifstream>(m_filename);

    if (m_stream->fail())
    {
        throw OpenError(m_filename);
    }
}

OpenError::OpenError(const std::string& filename)
    : m_what(std::string("Cannot open file: ") + filename)
{
}

const char* OpenError::what() const noexcept
{
    return m_what.c_str();
}
