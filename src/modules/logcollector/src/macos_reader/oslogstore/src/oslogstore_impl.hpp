#include "oslogstore_dependencies.hpp"

#import <Foundation/Foundation.h>
#import <OSLog/OSLog.h>

#import <exception>
#import <memory>
#import <string>
#import <vector>

class OSLogStoreWrapperImpl
{
public:
    /// @brief Constructor
    /// @details Initializes the log store and log processor
    /// to their default implementations.
    OSLogStoreWrapperImpl()
        : m_logStore(std::make_unique<OSLogStoreImpl>())
        , m_logProcessor(std::make_unique<LogProcessorImpl>())
    {
    }

    /// @brief SetLogStore
    /// @param logStore The log store to use.
    /// @details Intended for testing.
    void SetLogStore(std::unique_ptr<IOSLogStore> logStore)
    {
        m_logStore = std::move(logStore);
    }

    /// @brief SetLogProcessor
    /// @param logProcessor The log processor to use.
    /// @details Intended for testing.
    void SetLogProcessor(std::unique_ptr<ILogProcessor> logProcessor)
    {
        m_logProcessor = std::move(logProcessor);
    }

    /// @brief AllEntries
    /// @param startTimeSeconds The start time in seconds since the epoch, must be greater than 0.
    /// @param query The query to filter the log entries.
    /// @param logLevel The log level to filter the log entries.
    /// @return A vector of log entries.
    /// @details Retrieves all log entries from the log store that match the query and log level.
    /// If the start time is less than or equal to 0, a runtime error is thrown.
    /// If the log store fails to create an enumerator, a runtime error is thrown.
    /// If the log processor fails to process the log entries, a runtime error is thrown.
    std::vector<IOSLogStoreWrapper::LogEntry>
    AllEntries(double startTimeSeconds, const std::string& query, IOSLogStoreWrapper::LogLevel logLevel)
    {
        if (startTimeSeconds <= 0)
        {
            throw std::invalid_argument("startTimeSeconds must be greater than 0");
        }

        std::vector<IOSLogStoreWrapper::LogEntry> entries;

        NSDate* startDate = [NSDate dateWithTimeIntervalSince1970:startTimeSeconds];
        NSMutableArray* subpredicates = [NSMutableArray array];

        if (!query.empty())
        {
            NSString* queryNsStr = [NSString stringWithUTF8String:query.c_str()];
            NSPredicate* queryPredicate = [NSPredicate predicateWithFormat:queryNsStr];
            [subpredicates addObject:queryPredicate];
        }

        NSPredicate* combinedPredicate = [NSCompoundPredicate andPredicateWithSubpredicates:subpredicates];

        std::string error;

        auto osLogEntries = m_logStore->GetOSLogEntries(startDate, combinedPredicate, error);

        if (!osLogEntries)
        {
            throw std::runtime_error("Failed to get OSLogEntries: " + error);
        }

        auto* rawEntries = osLogEntries->AllEntries();
        return m_logProcessor->ProcessEntries(rawEntries, logLevel);
    }

private:
    std::unique_ptr<IOSLogStore> m_logStore;
    std::unique_ptr<ILogProcessor> m_logProcessor;
};
