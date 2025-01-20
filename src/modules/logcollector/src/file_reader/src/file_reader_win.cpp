#include "file_reader.hpp"

#include <logcollector.hpp>
#include <logger.hpp>

#include <list>
#include <string>
#include <windows.h>

using namespace logcollector;

void FileReader::Reload(const std::function<void(Localfile&)>& callback)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(m_filePattern.c_str(), &findFileData);
    std::list<std::string> files;

    auto baseDir = m_filePattern.substr(0, m_filePattern.find_last_of("\\/"));

    if (!baseDir.empty() && baseDir.back() != '\\' && baseDir.back() != '/')
    {
        baseDir += '\\';
    }

    if (hFind == INVALID_HANDLE_VALUE)
    {
        LogTrace("No matches found for pattern: {}", m_filePattern);
        return;
    }

    do
    {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            files.push_back(baseDir + findFileData.cFileName);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    AddLocalfiles(files, callback);
    FindClose(hFind);
}
