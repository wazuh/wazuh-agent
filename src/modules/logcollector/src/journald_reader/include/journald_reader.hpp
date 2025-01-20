#pragma once

#include <journal_log.hpp>
#include <logcollector.hpp>
#include <reader.hpp>

#include <memory>
#include <regex>

namespace logcollector
{

    /// @brief Reader class for systemd journal entries
    ///
    /// This class implements journal reading functionality with filtering capabilities.
    /// It supports both single and multiple condition filtering with AND/OR logic.
    class JournaldReader : public IReader
    {
    public:
        /// @brief Constructs a new journal reader
        /// @param logcollector Reference to logcollector instance
        /// @param filters Group of filters to apply (AND logic between them)
        /// @param ignoreIfMissing Whether to ignore missing fields
        /// @param fileWait Time to wait between reads in milliseconds
        JournaldReader(Logcollector& logcollector, FilterGroup filters, bool ignoreIfMissing, std::time_t fileWait);

        /// @brief Runs the journal reader
        /// @return Awaitable for asynchronous operation
        Awaitable Run() override;

        /// @brief Stops the journal reader
        void Stop() override;

        /// @brief Gets human-readable description of current filters
        /// @return String describing current filters
        std::string GetFilterDescription() const;

    private:
        FilterGroup m_filters;                           ///< Active filters
        bool m_ignoreIfMissing;                          ///< Whether to ignore missing fields
        std::unique_ptr<JournalLog> m_journal;           ///< Journal interface
        std::chrono::milliseconds m_waitTime;            ///< Wait time between reads
        static constexpr size_t MAX_LINE_LENGTH = 16384; ///< Maximum message length
    };

} // namespace logcollector
