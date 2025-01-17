#include "journald_reader.hpp"
#include <logger.hpp>
#include <sstream>

namespace logcollector
{
JournaldReader::JournaldReader(Logcollector& logcollector,
                             FilterGroup filters,
                             bool ignoreIfMissing,
                             std::time_t fileWait)
    : IReader(logcollector)
    , m_filters(std::move(filters))
    , m_ignoreIfMissing(ignoreIfMissing)
    , m_journal(std::make_unique<JournalLog>())
    , m_waitTime(std::chrono::milliseconds(fileWait)) {

    LogInfo("Creating JournaldReader with {} filters", m_filters.size());
}

std::string JournaldReader::GetFilterDescription() const {
    std::ostringstream desc;
    desc << m_filters.size() << " conditions: ";
    for (const auto& filter : m_filters) {
        desc << "[" << filter.field << (filter.exact_match ? "=" : "~") << filter.value << "] ";
    }
    return desc.str();
}

Awaitable JournaldReader::Run() {
    try {
        LogInfo("Initializing journald reader with {}", GetFilterDescription());
        m_journal->Open();
        m_journal->AddFilterGroup(m_filters, m_ignoreIfMissing);

        try {
            m_journal->SeekTail();
        } catch (const JournalLogException& e) {
            LogError("Failed to seek journal: {}", e.what());
            co_return;
        }

        LogInfo("Journald reader started successfully");

        while (m_keepRunning.load()) {
            bool shouldWait = true;
            try {
                LogTrace("Checking for new journal entries...");
                FilterSet filterSet{m_filters};
                while (auto filteredMessage = m_journal->GetNextFilteredMessage(filterSet, m_ignoreIfMissing)) {
                    shouldWait = false;
                    auto& message = filteredMessage->message;
                    LogDebug("Found matching message for {}", GetFilterDescription());

                    if (message.length() > MAX_LINE_LENGTH) {
                        LogDebug("Truncating message of length {}", message.length());
                        message.resize(MAX_LINE_LENGTH);
                    }
                    m_logcollector.SendMessage(filteredMessage->fieldValue, message, "journald");
                }
            } catch (const JournalLogException& e) {
                LogError("Journal reading error: {}", e.what());
            }

            if (shouldWait) {
                co_await m_logcollector.Wait(m_waitTime);
            }
        }
    } catch (const JournalLogException& e) {
        LogError("Failed to initialize journal: {}", e.what());
    }
}

void JournaldReader::Stop() {
    m_journal->FlushFilters();
    m_keepRunning.store(false);
    LogInfo("Journald stopped.");
}
} // namespace logcollector
