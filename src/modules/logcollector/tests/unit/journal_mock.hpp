#pragma once

#include <gmock/gmock.h>
#include "journal_log.hpp"

namespace logcollector {

class JournalMock : public JournalLog {
public:
    MOCK_METHOD(void, Open, (), (override));
    MOCK_METHOD(bool, Next, (), (override));
    MOCK_METHOD(bool, Previous, (), (override));
    MOCK_METHOD(bool, SeekHead, (), (override));
    MOCK_METHOD(bool, SeekTail, (), (override));
    MOCK_METHOD(bool, SeekTimestamp, (uint64_t timestamp), (override));
    MOCK_METHOD(std::string, GetData, (const std::string& field), (const, override));
    MOCK_METHOD(uint64_t, GetTimestamp, (), (const, override));
    MOCK_METHOD(std::string, GetCursor, (), (const, override));
    MOCK_METHOD(bool, SeekCursor, (const std::string& cursor), (override));
    MOCK_METHOD(void, AddFilterGroup, (const FilterGroup& group, bool ignoreIfMissing), (override));
    MOCK_METHOD(std::optional<FilteredMessage>, GetNextFilteredMessage, (const FilterSet& filters, bool ignoreIfMissing), (override));
};

}
