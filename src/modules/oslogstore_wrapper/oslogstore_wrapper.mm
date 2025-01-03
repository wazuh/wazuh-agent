#include "oslogstore_wrapper.hpp"

#import <Foundation/Foundation.h>
#import <OSLog/OSLog.h>

#import <exception>

class OSLogStoreWrapper::Impl
{
public:
    Impl()
    {
        @autoreleasepool
        {
            NSError* error = nil;

            osLogStore = [OSLogStore localStoreAndReturnError:&error];

            if (error)
            {
                const std::string errorDescription = [[error localizedDescription] UTF8String];
                throw std::runtime_error("Failed to create OSLogStore: " + errorDescription);
            }

            if (osLogStore)
            {
                [osLogStore retain];
            }
        }
    }

    ~Impl()
    {
        if (osLogStore)
        {
            [osLogStore release];
        }
    }

    OSLogStore* osLogStore = nullptr;
};

OSLogStoreWrapper::OSLogStoreWrapper()
    : m_impl(std::make_unique<Impl>())
{
}

OSLogStoreWrapper::~OSLogStoreWrapper() = default;

OSLogStoreWrapper::Iterator OSLogStoreWrapper::Begin(const char* startTimeStr, const char* endTimeStr)
{
    @autoreleasepool
    {
        NSError* error = nil;

        NSDateFormatter* formatter = [[NSDateFormatter alloc] init];
        formatter.dateFormat = @"yyyy-MM-dd HH:mm:ss";
        NSDate* startDate = [formatter dateFromString:[NSString stringWithUTF8String:startTimeStr]];
        NSDate* endDate = [formatter dateFromString:[NSString stringWithUTF8String:endTimeStr]];

        OSLogPosition* position = [m_impl->osLogStore positionWithDate:startDate];
        NSPredicate* predicateTime = [NSPredicate predicateWithFormat:@"date < %@", endDate];
        OSLogEnumerator* enumerator = [m_impl->osLogStore entriesEnumeratorWithOptions:0
                                                                              position:position
                                                                             predicate:predicateTime
                                                                                 error:&error];
        if (error)
        {
            throw std::runtime_error("Failed to create OSLogEnumerator");
        }

        [enumerator retain];
        return Iterator(this, enumerator);
    }
}

OSLogStoreWrapper::Iterator OSLogStoreWrapper::End()
{
    return Iterator();
}

OSLogStoreWrapper::Iterator::Iterator(OSLogStoreWrapper* wrapper, void* osLogEnumerator)
    : m_wrapper(wrapper)
    , m_osLogEnumerator(osLogEnumerator)
{
    Advance();
}

OSLogStoreWrapper::Iterator::~Iterator()
{
    if (m_osLogEnumerator)
    {
        [(OSLogEnumerator*)m_osLogEnumerator release];
    }
}

void OSLogStoreWrapper::Iterator::Advance()
{
    @autoreleasepool
    {
        if (!m_osLogEnumerator)
        {
            m_currentLog = {};
            return;
        }

        OSLogEntry* logEntry = [(OSLogEnumerator*)m_osLogEnumerator nextObject];

        if (logEntry)
        {
            const char* rawDate = (logEntry.date != nil) ? [[logEntry.date description] UTF8String] : "";
            const char* rawMessage = (logEntry.composedMessage != nil) ? [logEntry.composedMessage UTF8String] : "";

            const std::string dateStr = rawDate;
            const std::string messageStr = rawMessage;
            m_currentLog = dateStr + " " + messageStr;
        }
        else
        {
            m_osLogEnumerator = nullptr;
        }
    }
}

OSLogStoreWrapper::Iterator::Reference OSLogStoreWrapper::Iterator::operator*() const
{
    return m_currentLog;
}

OSLogStoreWrapper::Iterator::Pointer OSLogStoreWrapper::Iterator::operator->() const
{
    return &m_currentLog;
}

OSLogStoreWrapper::Iterator& OSLogStoreWrapper::Iterator::operator++()
{
    Advance();
    return *this;
}

bool OSLogStoreWrapper::Iterator::operator==(const Iterator& other) const
{
    return m_osLogEnumerator == other.m_osLogEnumerator;
}

bool OSLogStoreWrapper::Iterator::operator!=(const Iterator& other) const
{
    return !(*this == other);
}
