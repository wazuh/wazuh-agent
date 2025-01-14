#pragma once

#include "reader.hpp"
#include "journal_log.hpp"
#include <regex>
#include <memory>

namespace logcollector {

class JournaldReader : public IReader {
public:
    JournaldReader(Logcollector& logcollector,
                  FilterGroup filters,
                  bool ignoreIfMissing,
                  std::time_t fileWait);
    Awaitable Run() override;
    void Stop() override;
    std::string GetFilterDescription() const;

private:
    FilterGroup m_filters;
    bool m_ignoreIfMissing;
    std::unique_ptr<JournalLog> m_journal;
    std::chrono::milliseconds m_waitTime;
    static constexpr size_t MAX_LINE_LENGTH = 16384; // OS_MAXSTR - OS_LOG_HEADER
};

}
