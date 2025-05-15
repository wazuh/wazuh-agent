#pragma once

#include <journal_log.hpp>
#include <reader.hpp>

#include <chrono>
#include <functional>
#include <memory>
#include <regex>
#include <string>

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
        /// @param pushMessageFunc Push message function
        /// @param waitFunc Wait function
        /// @param filters Group of filters to apply (AND logic between them)
        /// @param ignoreIfMissing Whether to ignore missing fields
        /// @param fileWait Time to wait between reads in milliseconds
        JournaldReader(
            std::function<void(const std::string& location, const std::string& log, const std::string& collectorType)>
                pushMessageFunc,
            std::function<Awaitable(std::chrono::milliseconds)> waitFunc,
            FilterGroup filters,
            bool ignoreIfMissing,
            std::time_t fileWait);

        /// @copydoc IReader::Run
        Awaitable Run() override;

        /// @copydoc IReader::Stop
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
