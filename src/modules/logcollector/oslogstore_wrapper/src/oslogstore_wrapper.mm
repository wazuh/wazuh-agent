#include "oslogstore_wrapper.hpp"

#import <Foundation/Foundation.h>
#import <OSLog/OSLog.h>

#import <exception>

std::vector<OSLogStoreWrapper::LogEntry>
OSLogStoreWrapper::AllEntries(const double startTimeSeconds, const std::string& query, const LogLevel logLevel)
{
    std::vector<LogEntry> entries;

    @autoreleasepool
    {
        NSError* error = nil;

        OSLogStore* osLogStore = [OSLogStore localStoreAndReturnError:&error];

        if (error)
        {
            const std::string errorDescription = [[error localizedDescription] UTF8String];
            throw std::runtime_error("Failed to create OSLogStore: " + errorDescription);
        }

        NSDate* startDate = [NSDate dateWithTimeIntervalSince1970:startTimeSeconds];
        NSMutableArray* subpredicates = [NSMutableArray array];

        if (!query.empty())
        {
            NSString* queryNsStr = [NSString stringWithUTF8String:query.c_str()];
            NSPredicate* queryPredicate = [NSPredicate predicateWithFormat:queryNsStr];
            [subpredicates addObject:queryPredicate];
        }

        NSPredicate* combinedPredicate = [NSCompoundPredicate andPredicateWithSubpredicates:subpredicates];

        OSLogPosition* position = [osLogStore positionWithDate:startDate];
        OSLogEnumerator* enumerator = [osLogStore entriesEnumeratorWithOptions:0
                                                                      position:position
                                                                     predicate:combinedPredicate
                                                                         error:&error];

        if (error)
        {
            throw std::runtime_error("Failed to create OSLogEnumerator");
        }

        NSArray<OSLogEntry*>* logEntries = [enumerator allObjects];
        if (logEntries)
        {
            for (OSLogEntry* logEntry in logEntries)
            {
                if ([logEntry respondsToSelector:@selector(level)])
                {
                    OSLogEntryLog* typedLog = static_cast<OSLogEntryLog*>(logEntry);
                    if (static_cast<LogLevel>(typedLog.level) >= logLevel)
                    {
                        NSTimeInterval timeInterval = [logEntry.date timeIntervalSince1970];
                        NSString* baseDate = [logEntry.date description];
                        const char* rawDate = [baseDate UTF8String];
                        const char* rawMsg = logEntry.composedMessage ? [logEntry.composedMessage UTF8String] : "";
                        entries.emplace_back(timeInterval, rawDate, rawMsg);
                    }
                }
            }
        }
    }
    return entries;
}
