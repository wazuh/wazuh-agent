#pragma once

#include <oslogstore_wrapper.hpp>

#import <Foundation/Foundation.h>
#import <OSLog/OSLog.h>

#include <memory>
#include <string>
#include <vector>

@class OSLogStore;
@class OSLogEnumerator;
@class NSPredicate;
@class NSDate;
@class OSLogEntry;
@class OSLogEntryLog;

/// @brief Wrapper for OSLogStore.
/// @details This class is used to create an enumerator to iterate over the log entries.
class IOSLogStore
{
public:
    virtual ~IOSLogStore() = default;
    virtual std::unique_ptr<class IOSLogEntries>
    GetOSLogEntries(NSDate* startDate, NSPredicate* predicate, std::string& error) = 0;
};

/// @brief Wrapper for a function that returns all log entries
class IOSLogEntries
{
public:
    virtual ~IOSLogEntries() = default;
    virtual NSArray<OSLogEntry*>* AllEntries() = 0;
};

/// @brief Wrapper for a function that processes log entries
class ILogProcessor
{
public:
    virtual ~ILogProcessor() = default;
    virtual std::vector<IOSLogStoreWrapper::LogEntry> ProcessEntries(NSArray<OSLogEntry*>* entries,
                                                                     IOSLogStoreWrapper::LogLevel logLevel) = 0;
};

/// @brief Implementation for IOSLogEntries
/// @details This class returns all entries from an OSLogEnumerator
class OSLogEntriesImpl : public IOSLogEntries
{
public:
    /// @brief Constructor
    /// @param enumerator Enumerator to iterate over log entries
    OSLogEntriesImpl(OSLogEnumerator* enumerator)
        : m_enumerator(enumerator)
    {
        [m_enumerator retain];
    }

    /// @brief Destructor
    ~OSLogEntriesImpl() override
    {
        if (m_enumerator)
        {
            [m_enumerator release];
        }
    }

    /// @brief Function to return all log entries
    /// @return NSArray<OSLogEntry*>* with log entries
    /// @details This function returns all log entries from the enumerator
    /// The OSLogEnumerator needs to be exhausted always otherwise
    /// a thread will be blocked on an apple logstream
    NSArray<OSLogEntry*>* AllEntries() override
    {
        if (!m_enumerator)
        {
            return nil;
        }
        return [m_enumerator allObjects];
    }

private:
    OSLogEnumerator* m_enumerator;
};

/// @brief Implementation for ILogProcessor
class LogProcessorImpl : public ILogProcessor
{
public:
    /// @brief Function to process log entries
    /// @param entries NSArray<OSLogEntry*>* with log entries
    /// @param logLevel LogLevel to filter log entries
    /// @return std::vector<IOSLogStoreWrapper::LogEntry> with processed log entries
    /// @details This function processes OSLogEntries as IOSLogStoreWrapper::LogEntries
    /// and returns only the ones with a log level equal or higher than the given log level
    std::vector<IOSLogStoreWrapper::LogEntry> ProcessEntries(NSArray<OSLogEntry*>* entries,
                                                             IOSLogStoreWrapper::LogLevel logLevel) override
    {
        std::vector<IOSLogStoreWrapper::LogEntry> processedEntries;

        @autoreleasepool
        {
            if (!entries)
            {
                return processedEntries;
            }

            for (OSLogEntry* logEntry in entries)
            {
                if ([logEntry respondsToSelector:@selector(level)])
                {
                    OSLogEntryLog* typedLog = static_cast<OSLogEntryLog*>(logEntry);

                    if (static_cast<IOSLogStoreWrapper::LogLevel>(typedLog.level) >= logLevel)
                    {
                        NSTimeInterval timeInterval = [logEntry.date timeIntervalSince1970];
                        NSString* baseDate = [logEntry.date description];

                        const char* rawDate = [baseDate UTF8String];
                        const char* rawMsg = logEntry.composedMessage ? [logEntry.composedMessage UTF8String] : "";

                        processedEntries.push_back(IOSLogStoreWrapper::LogEntry {
                            static_cast<double>(timeInterval), std::string(rawDate), std::string(rawMsg)});
                    }
                }
            }
        }

        return processedEntries;
    }
};

/// @brief Implementation for IOSLogStore
class OSLogStoreImpl : public IOSLogStore
{
public:
    /// @brief Function to get log entries
    /// @param startDate Start date to get log entries
    /// @param predicate Predicate to filter log entries
    /// @param error Error message
    /// @return std::unique_ptr<IOSLogEntries> with log entries
    /// @details This function returns all log entries from the OSLogStore that match the given predicate if any.
    /// If an error occurs, the error message is returned.
    std::unique_ptr<IOSLogEntries>
    GetOSLogEntries(NSDate* startDate, NSPredicate* predicate, std::string& error) override
    {
        std::unique_ptr<IOSLogEntries> entries;

        @autoreleasepool
        {
            NSError* nsError = nil;
            OSLogStore* store = [OSLogStore localStoreAndReturnError:&nsError];

            if (nsError || !store)
            {
                error = [[nsError localizedDescription] UTF8String];
                return nullptr;
            }

            OSLogPosition* position = [store positionWithDate:startDate];
            OSLogEnumerator* enumerator =
                [store entriesEnumeratorWithOptions:0 position:position predicate:predicate error:&nsError];

            if (nsError || !enumerator)
            {
                error = [[nsError localizedDescription] UTF8String];
                return nullptr;
            }

            entries = std::make_unique<OSLogEntriesImpl>(enumerator);
        }

        return entries;
    }
};
