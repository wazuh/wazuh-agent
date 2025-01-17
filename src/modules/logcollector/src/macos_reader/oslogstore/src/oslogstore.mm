#include "oslogstore.hpp"
#include "oslogstore_dependencies.hpp"
#include "oslogstore_impl.hpp"

#import <Foundation/Foundation.h>
#import <OSLog/OSLog.h>

#import <memory>
#import <string>

OSLogStoreWrapper::OSLogStoreWrapper() = default;
OSLogStoreWrapper::~OSLogStoreWrapper() = default;

std::vector<IOSLogStoreWrapper::LogEntry>
OSLogStoreWrapper::AllEntries(const double startTimeSeconds, const std::string& query, const LogLevel logLevel)
{
    if (!m_impl)
    {
        m_impl = std::make_unique<OSLogStoreWrapperImpl>();
    }
    return m_impl->AllEntries(startTimeSeconds, query, logLevel);
}
