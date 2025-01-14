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
    virtual ~JournalLog();

    virtual void Open();
    virtual bool Next();
    virtual bool Previous();
    virtual bool SeekHead();
    virtual bool SeekTail();
    virtual bool SeekTimestamp(uint64_t timestamp);

    virtual std::string GetData(const std::string& field) const;
    virtual uint64_t GetTimestamp() const;

    virtual std::string GetCursor() const;
    virtual bool SeekCursor(const std::string& cursor);

    struct FilteredMessage {
        std::string fieldValue;
        std::string message;
    };

    virtual void AddFilterGroup(const FilterGroup& group, bool ignoreIfMissing);
    virtual std::optional<FilteredMessage> GetNextFilteredMessage(const FilterSet& filters, bool ignoreIfMissing);

    void FlushFilters();

    static bool ValidateFilter(const JournalFilter& filter) {
        return !filter.field.empty() && !filter.value.empty();
    }

    virtual uint64_t GetOldestTimestamp() const;
    virtual void UpdateTimestamp();
    virtual bool SeekMostRecent();
    virtual bool NextNewest();
    virtual bool SeekToTimestamp(uint64_t timestamp);
    virtual bool CursorValid(const std::string& cursor) const;

private:
    struct sd_journal* m_journal;
    uint64_t m_currentTimestamp{0};
    static uint64_t GetEpochTime();
    void ThrowIfError(int result, const std::string& operation) const;

    bool m_hasActiveFilters{false};
    FilterSet m_filters;
    bool ApplyFilterSet(const FilterSet& filters, bool ignoreIfMissing);
    bool ApplyFilterGroup(const FilterGroup& group, bool ignoreIfMissing) const;

    bool ProcessJournalEntry(const FilterSet& filters, bool ignoreIfMissing, FilteredMessage& message) const;
};

}
