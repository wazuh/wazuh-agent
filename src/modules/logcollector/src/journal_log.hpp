#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include <chrono>
#include <regex>
#include <optional>
#include <string_view>
#include <vector>
#include <algorithm>

// Forward declaration for sd_journal
struct sd_journal;

namespace logcollector {

// Add new filter structures before the JournalLog class
struct JournalFilter {
    std::string field;
    std::string value;
    bool exact_match{true};

    std::vector<std::string_view> GetValueViews() const {
        std::vector<std::string_view> values;
        std::string_view sv(value);
        size_t pos = 0;

        while ((pos = sv.find('|')) != std::string_view::npos) {
            values.push_back(sv.substr(0, pos));
            sv.remove_prefix(pos + 1);
        }
        if (!sv.empty()) {
            values.push_back(sv);
        }
        return values;
    }

    bool Matches(const std::string& fieldValue) const {
        auto values = GetValueViews();
        return std::any_of(values.begin(), values.end(),
            [&fieldValue, this](const auto& val) {
                return exact_match ?
                    fieldValue == val :
                    fieldValue.find(val) != std::string::npos;
            });
    }
};

using FilterGroup = std::vector<JournalFilter>;  // AND conditions
using FilterSet = std::vector<FilterGroup>;      // OR conditions

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

    std::string GetData(const std::string& field) const;
    uint64_t GetTimestamp() const;
    uint64_t GetOldestTimestamp() const;

    std::string GetCursor() const;
    bool SeekCursor(const std::string& cursor);
    bool CursorValid(const std::string& cursor) const;

    struct FilteredMessage {
        std::string fieldValue;
        std::string message;
    };

    void AddFilterGroup(const FilterGroup& group, bool ignoreIfMissing);
    std::optional<FilteredMessage> GetNextFilteredMessage(const FilterSet& filters, bool ignoreIfMissing);

    void FlushFilters();

    static bool ValidateFilter(const JournalFilter& filter) {
        return !filter.field.empty() && !filter.value.empty();
    }

private:
    struct sd_journal* m_journal;
    uint64_t m_currentTimestamp;
    static uint64_t GetEpochTime();
    void ThrowIfError(int result, const std::string& operation) const;

    bool m_hasActiveFilters{false};
    FilterSet m_filters;
    bool ApplyFilterSet(const FilterSet& filters, bool ignoreIfMissing);
    bool ApplyFilterGroup(const FilterGroup& group, bool ignoreIfMissing) const;

    bool ProcessJournalEntry(const FilterSet& filters, bool ignoreIfMissing, FilteredMessage& message) const;
};

}
