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

/// @brief Structure to define journal filtering criteria
struct JournalFilter {
    std::string field;      ///< Field name to filter on
    std::string value;      ///< Value or pattern to match (supports OR with '|')
    bool exact_match{true}; ///< Whether to perform exact matching or substring matching

    /// @brief Splits the value string into multiple values based on '|' separator
    /// @return Vector of string_views representing individual values
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

    /// @brief Checks if a field value matches any of the filter values
    /// @param fieldValue The value to check against filter values
    /// @return true if matches, false otherwise
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

/// @brief Group of filters combined with AND logic
using FilterGroup = std::vector<JournalFilter>;
/// @brief Set of filter groups combined with OR logic
using FilterSet = std::vector<FilterGroup>;

/// @brief Exception class for journal-related errors
class JournalLogException : public std::runtime_error {
public:
    explicit JournalLogException(const std::string& message) : std::runtime_error(message) {}
};

/// @brief Class for interacting with systemd journal
///
/// This class provides an interface to read and filter systemd journal entries.
/// It supports complex filtering with AND/OR logic and handles journal rotation.
class JournalLog {
public:
    /// @brief Message structure containing filtered journal entry data
    struct FilteredMessage {
        std::string fieldValue; ///< Value of the matched field
        std::string message;    ///< Journal message content
    };

    JournalLog();
    virtual ~JournalLog();

    /// @brief Opens the systemd journal
    virtual void Open();

    /// @brief Moves to next journal entry
    /// @return true if successful, false if no more entries
    virtual bool Next();

    /// @brief Moves to previous journal entry
    /// @return true if successful, false if no previous entry
    virtual bool Previous();

    /// @brief Seeks to the start of the journal
    virtual bool SeekHead();

    /// @brief Seeks to the end of the journal
    virtual bool SeekTail();

    /// @brief Seeks to a specific timestamp in the journal
    /// @param timestamp Timestamp in microseconds since epoch
    virtual bool SeekTimestamp(uint64_t timestamp);

    /// @brief Retrieves field data from current journal entry
    /// @param field Field name to retrieve
    /// @return Field value as string
    /// @throw JournalLogException if field not found
    virtual std::string GetData(const std::string& field) const;

    /// @brief Gets timestamp of current journal entry
    /// @return Timestamp in microseconds since epoch
    virtual uint64_t GetTimestamp() const;

    /// @brief Adds a group of filters with AND logic between them
    /// @param group Group of filters to add
    /// @param ignoreIfMissing Whether to ignore missing fields
    virtual void AddFilterGroup(const FilterGroup& group, bool ignoreIfMissing);

    /// @brief Gets next message that matches current filters
    /// @param filters Set of filter groups to apply
    /// @param ignoreIfMissing Whether to ignore missing fields
    /// @return Optional containing filtered message if found
    virtual std::optional<FilteredMessage> GetNextFilteredMessage(
        const FilterSet& filters, bool ignoreIfMissing);

    /// @brief Clears all active filters
    void FlushFilters();

    /// @brief Validates a filter configuration
    /// @param filter Filter to validate
    /// @return true if filter is valid, false otherwise
    static bool ValidateFilter(const JournalFilter& filter) {
        return !filter.field.empty() && !filter.value.empty();
    }

    virtual std::string GetCursor() const;
    virtual bool SeekCursor(const std::string& cursor);

    virtual uint64_t GetOldestTimestamp() const;
    virtual void UpdateTimestamp();
    virtual bool SeekMostRecent();
    virtual bool NextNewest();
    virtual bool SeekToTimestamp(uint64_t timestamp);
    virtual bool CursorValid(const std::string& cursor) const;

private:
    struct sd_journal* m_journal;          ///< Pointer to journal structure
    uint64_t m_currentTimestamp{0};        ///< Current entry timestamp
    bool m_hasActiveFilters{false};        ///< Whether filters are currently active
    FilterSet m_filters;                   ///< Currently active filters

    /// @brief Gets current epoch time in microseconds
    static uint64_t GetEpochTime();

    /// @brief Checks and throws on journal operation errors
    /// @param result Operation result code
    /// @param operation Operation description for error message
    void ThrowIfError(int result, const std::string& operation) const;

    /// @brief Applies filter set with OR logic between groups
    bool ApplyFilterSet(const FilterSet& filters, bool ignoreIfMissing);

    /// @brief Applies filter group with AND logic between filters
    bool ApplyFilterGroup(const FilterGroup& group, bool ignoreIfMissing) const;

    /// @brief Processes current journal entry
    bool ProcessJournalEntry(const FilterSet& filters,
                           bool ignoreIfMissing,
                           FilteredMessage& message) const;
};

}
