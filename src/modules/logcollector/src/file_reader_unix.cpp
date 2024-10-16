#include "file_reader.hpp"
#include <logcollector.hpp>
#include <logger.hpp>
#include <glob.h>

using namespace logcollector;
using namespace std;

void FileReader::Reload(const function<void (Localfile &)> & callback) {
    glob_t globResult;

    int ret = glob(m_filePattern.c_str(), 0, nullptr, &globResult);

    if (ret != 0) {
        if (ret == GLOB_NOMATCH) {
            LogTrace("No matches found for pattern: {}", m_filePattern);
        } else {
            LogWarn("Cannot use glob with pattern: {}", m_filePattern);
        }

        globfree(&globResult);
        return;
    }

    list<string> localfiles;
    auto paths = span<char *>(globResult.gl_pathv, globResult.gl_pathc);

    for (auto& path : paths) {
        localfiles.emplace_back(path);
    }

    AddLocalfiles(localfiles, callback);
    globfree(&globResult);
}
