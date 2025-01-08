#pragma once

#include "reader.hpp"
#include "journal_log.hpp"
#include <regex>
#include <memory>

namespace logcollector {

class JournaldReader : public IReader {
public:
    JournaldReader(Logcollector& logcollector,
                  std::string field,
                  const std::string& regex,  // Changed to const reference
                  bool ignoreIfMissing);
    Awaitable Run() override;
    void Stop() override;

private:
    std::string m_field;
    std::regex m_pattern;
    bool m_ignoreIfMissing;
    std::unique_ptr<JournalLog> m_journal;
    static constexpr std::chrono::milliseconds POLL_INTERVAL{500};
    static constexpr size_t MAX_LINE_LENGTH = 16384; // OS_MAXSTR - OS_LOG_HEADER
};

}
