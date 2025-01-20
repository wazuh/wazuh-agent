#include "journal_log.hpp"

#include <algorithm>
#include <cstring>
#include <logger.hpp>
#include <ranges>
#include <systemd/sd-journal.h>

JournalLog::JournalLog()
    : m_journal(nullptr)
{
}

JournalLog::~JournalLog()
{
    if (m_journal)
    {
        sd_journal_close(m_journal);
    }
}

void JournalLog::Open()
{
    int ret = sd_journal_open(&m_journal, SD_JOURNAL_LOCAL_ONLY);
    ThrowIfError(ret, "open journal");
    LogInfo("Journal opened successfully");
}

bool JournalLog::Next()
{
    int ret = sd_journal_next(m_journal);
    ThrowIfError(ret, "next entry");
    return ret > 0;
}

bool JournalLog::Previous()
{
    int ret = sd_journal_previous(m_journal);
    ThrowIfError(ret, "previous entry");
    return ret > 0;
}

bool JournalLog::SeekHead()
{
    int ret = sd_journal_seek_head(m_journal);
    ThrowIfError(ret, "seek head");
    return ret >= 0;
}

bool JournalLog::SeekTail()
{
    int ret = sd_journal_seek_tail(m_journal);
    ThrowIfError(ret, "seek tail");

    ret = sd_journal_previous(m_journal);
    if (ret <= 0)
    {
        LogWarn("No entries found in journal after seeking to tail");
        return false;
    }

    LogTrace("Successfully seeked to tail of journal");
    return true;
}

bool JournalLog::SeekTimestamp(uint64_t timestamp)
{
    int ret = sd_journal_seek_realtime_usec(m_journal, timestamp);
    ThrowIfError(ret, "seek timestamp");
    return ret >= 0;
}

std::string JournalLog::GetData(const std::string& field) const
{
    const void* data = nullptr;
    size_t length = 0;
    int ret = sd_journal_get_data(m_journal, field.c_str(), &data, &length);
    if (ret == -ENOENT)
    {
        throw JournalLogException("Field not present in current journal entry");
    }
    ThrowIfError(ret, "get data");

    const char* str = static_cast<const char*>(data);
    std::string_view full_str(str, length);
    size_t prefix_len = field.length() + 1;

    return std::string(full_str.substr(prefix_len));
}

uint64_t JournalLog::GetTimestamp() const
{
    uint64_t timestamp = 0;
    int ret = sd_journal_get_realtime_usec(m_journal, &timestamp);
    ThrowIfError(ret, "get timestamp");
    return timestamp;
}

uint64_t JournalLog::GetOldestTimestamp() const
{
    uint64_t timestamp = 0;
    int ret = sd_journal_get_cutoff_realtime_usec(m_journal, &timestamp, nullptr);
    ThrowIfError(ret, "get oldest timestamp");
    return timestamp;
}

void JournalLog::ThrowIfError(int result, const std::string& operation) const
{
    if (result < 0)
    {
        throw JournalLogException("Failed to " + operation + ": " + strerror(-result));
    }
}

void JournalLog::UpdateTimestamp()
{
    static bool failed_logged = false;
    try
    {
        m_currentTimestamp = GetTimestamp();
    }
    catch (const JournalLogException& e)
    {
        m_currentTimestamp = GetEpochTime();
        if (!failed_logged)
        {
            failed_logged = true;
            LogWarn("Failed to read timestamp: {}", e.what());
        }
    }
}

bool JournalLog::SeekMostRecent()
{
    if (!SeekTail())
    {
        return false;
    }

    if (Previous())
    {
        UpdateTimestamp();
        return true;
    }
    return false;
}

bool JournalLog::NextNewest()
{
    if (Next())
    {
        UpdateTimestamp();
        return true;
    }
    return false;
}

bool JournalLog::SeekToTimestamp(uint64_t timestamp)
{
    if (timestamp == 0 || timestamp > GetEpochTime())
    {
        LogWarn("Invalid timestamp {} (in future), seeking most recent entry", timestamp);
        return SeekMostRecent();
    }

    try
    {
        uint64_t oldest {0};
        oldest = GetOldestTimestamp();
        if (timestamp < oldest)
        {
            LogWarn("Timestamp {} is older than oldest available {}, adjusting", timestamp, oldest);
            timestamp = oldest;
        }
    }
    catch (const JournalLogException& e)
    {
        LogWarn("Failed to read oldest timestamp: {}", e.what());
    }

    if (!SeekTimestamp(timestamp))
    {
        return false;
    }

    return NextNewest();
}

uint64_t JournalLog::GetEpochTime()
{
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count());
}

std::string JournalLog::GetCursor() const
{
    char* rawCursor = nullptr;
    int ret = sd_journal_get_cursor(m_journal, &rawCursor);
    ThrowIfError(ret, "get cursor");

    if (!rawCursor)
    {
        return {};
    }

    auto cursorGuard = std::unique_ptr<char, decltype(&std::free)>(rawCursor, std::free);
    return cursorGuard.get();
}

bool JournalLog::SeekCursor(const std::string& cursor)
{
    int ret = sd_journal_seek_cursor(m_journal, cursor.c_str());
    if (ret < 0)
    {
        LogWarn("Failed to seek to cursor: {}", strerror(-ret));
        return false;
    }
    return Next();
}

bool JournalLog::CursorValid(const std::string& cursor) const
{
    int ret = sd_journal_test_cursor(m_journal, cursor.c_str());
    return ret > 0;
}

void JournalLog::AddFilterGroup(const FilterGroup& group, bool ignoreIfMissing)
{
    if (group.empty())
    {
        LogWarn("Attempted to add empty filter group");
        return;
    }

    LogInfo("Adding filter group with {} conditions", group.size());
    if (!std::ranges::all_of(group, ValidateFilter))
    {
        throw JournalLogException("Invalid filter configuration");
    }

    for (const auto& filter : group)
    {
        for (const auto& value : filter.GetValueViews())
        {
            std::string match = filter.field + (filter.exact_match ? "=" : "~") + std::string(value);
            LogDebug("Adding journal match: {}", match);

            int ret = sd_journal_add_match(m_journal, match.c_str(), 0);
            if (ret < 0)
            {
                if (!ignoreIfMissing)
                {
                    ThrowIfError(ret, "add filter match");
                }
                LogWarn("Failed to add journal match {}: {}", match, strerror(-ret));
                continue;
            }
        }

        // Add OR condition between values of the same field
        if (filter.GetValueViews().size() > 1)
        {
            ThrowIfError(sd_journal_add_disjunction(m_journal), "add filter disjunction for field values");
        }
    }

    ThrowIfError(sd_journal_add_conjunction(m_journal), "add filter conjunction");
    m_hasActiveFilters = true;
    LogInfo("Filter group added successfully");
}

void JournalLog::FlushFilters()
{
    if (m_hasActiveFilters)
    {
        sd_journal_flush_matches(m_journal);
        m_hasActiveFilters = false;
    }
}

bool JournalLog::ApplyFilterGroup(const FilterGroup& group, bool ignoreIfMissing) const
{
    return std::all_of(group.begin(),
                       group.end(),
                       [this, ignoreIfMissing](const auto& filter)
                       {
                           try
                           {
                               std::string fieldValue = GetData(filter.field);
                               return filter.Matches(fieldValue);
                           }
                           catch (const JournalLogException&)
                           {
                               if (!ignoreIfMissing)
                               {
                                   LogTrace("Field {} not present in entry, skipping...", filter.field);
                               }
                               return false;
                           }
                       });
}

bool JournalLog::ApplyFilterSet(const FilterSet& filters, bool ignoreIfMissing)
{
    return std::any_of(filters.begin(),
                       filters.end(),
                       [this, ignoreIfMissing](const auto& group) { return ApplyFilterGroup(group, ignoreIfMissing); });
}

bool JournalLog::ProcessJournalEntry(const FilterSet&, bool ignoreIfMissing, FilteredMessage& message) const
{
    try
    {
        message.message = GetData("MESSAGE");
        try
        {
            message.fieldValue = GetData("_SYSTEMD_UNIT");
        }
        catch (const JournalLogException&)
        {
            message.fieldValue = "unknown";
        }
        return true;
    }
    catch (const JournalLogException& e)
    {
        if (!ignoreIfMissing)
        {
            LogError("Failed to process journal entry: {}", e.what());
        }
        return false;
    }
}

std::optional<JournalLog::FilteredMessage> JournalLog::GetNextFilteredMessage(const FilterSet& filters,
                                                                              bool ignoreIfMissing)
{

    if (!m_hasActiveFilters)
    {
        LogWarn("No active filters when trying to get filtered message");
        return std::nullopt;
    }

    while (Next())
    {
        FilteredMessage message;
        if (ApplyFilterSet(filters, ignoreIfMissing) && ProcessJournalEntry(filters, ignoreIfMissing, message))
        {
            return message;
        }
        UpdateTimestamp();
    }
    return std::nullopt;
}
