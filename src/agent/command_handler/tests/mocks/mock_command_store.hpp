#pragma once

#include <gmock/gmock.h>

#include <command_entry.hpp>
#include <icommand_store.hpp>

#include <optional>
#include <string>
#include <vector>

namespace command_store
{
    class MockCommandStore : public ICommandStore
    {
    public:
        MOCK_METHOD(bool, Clear, (), (override));
        MOCK_METHOD(int, GetCount, (), (override));
        MOCK_METHOD(bool, StoreCommand, (const module_command::CommandEntry& cmd), (override));
        MOCK_METHOD(bool, DeleteCommand, (const std::string& id), (override));
        MOCK_METHOD(std::optional<module_command::CommandEntry>, GetCommand, (const std::string& id), (override));
        MOCK_METHOD(std::optional<std::vector<module_command::CommandEntry>>,
                    GetCommandByStatus,
                    (const module_command::Status& status),
                    (override));
        MOCK_METHOD(bool, UpdateCommand, (const module_command::CommandEntry& cmd), (override));
    };
} // namespace command_store
