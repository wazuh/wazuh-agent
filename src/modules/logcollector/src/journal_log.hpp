#pragma once

#include <systemd/sd-journal.h>
#include <string>
#include <memory>
#include <stdexcept>
#include <chrono>
#include <regex>
#include <optional>
#include <string_view>

namespace logcollector {

class JournalLogException : public std::runtime_error {
public:
    explicit JournalLogException(const std::string& message) : std::runtime_error(message) {}
};

class JournalLog {
public:
    JournalLog();
    ~JournalLog();

    void Open();
    bool Next();
    bool Previous();
    bool SeekHead();
    bool SeekTail();
    bool SeekTimestamp(uint64_t timestamp);

    void UpdateTimestamp();
    bool SeekMostRecent();
    bool SeekToTimestamp(uint64_t timestamp);
    bool NextNewest();
    bool NextNewestFiltered();
    bool ApplyFilter() const;

    std::string GetData(const std::string& field) const;
    uint64_t GetTimestamp() const;
    uint64_t GetOldestTimestamp() const;
    uint64_t GetCurrentTimestamp() const { return m_currentTimestamp; }

    struct FilteredMessage {
        std::string fieldValue;
        std::string message;
    };

    std::optional<FilteredMessage> GetNextFilteredMessage(
        const std::string& field,
        const std::regex& pattern,
        bool ignoreIfMissing);

private:
    sd_journal* m_journal;
    uint64_t m_currentTimestamp;
    static uint64_t GetEpochTime();
    void ThrowIfError(int result, const std::string& operation) const;
};

}
