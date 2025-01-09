#include "journald_reader.hpp"
#include <logger.hpp>

using namespace logcollector;

JournaldReader::JournaldReader(Logcollector& logcollector,
                             std::string field,
                             const std::string& regex,
                             bool ignoreIfMissing,
                             std::time_t fileWait)
    : IReader(logcollector)
    , m_field(std::move(field))
    , m_pattern(regex)
    , m_ignoreIfMissing(ignoreIfMissing)
    , m_journal(std::make_unique<JournalLog>())
    , m_waitTime(std::chrono::milliseconds(fileWait)) {}

Awaitable JournaldReader::Run() {
    try {
        m_journal->Open();
        LogInfo("Journald opened successfully");

        try {
            m_journal->SeekHead();
            LogInfo("Successfully seeked to head of journal");
            m_journal->SeekTail();
            LogInfo("Successfully seeked to tail of journal");
        } catch (const JournalLogException& e) {
            LogError("Failed to seek journal: {}", e.what());
            co_return;
        }

        LogInfo("Journald started with field: {}", m_field);

        while (m_keepRunning.load()) {
            bool shouldWait = true;
            try {
                LogDebug("Checking for new journal entries...");
                while (auto filteredMessage = m_journal->GetNextFilteredMessage(
                    m_field, m_pattern, m_ignoreIfMissing)) {
                    shouldWait = false;
                    auto& message = filteredMessage->message;
                    LogDebug("Found matching message: {} for field: {} with value: {}",
                           message, m_field, filteredMessage->fieldValue);

                    if (message.length() > MAX_LINE_LENGTH) {
                        LogDebug("Truncating message of length {}", message.length());
                        message.resize(MAX_LINE_LENGTH);
                    }
                    m_logcollector.SendMessage(m_field, message, "journald");
                }

                if (shouldWait) {
                    LogTrace("No new matching entries found, waiting...");
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
    m_keepRunning.store(false);
    LogInfo("Journald stopped.");
}
