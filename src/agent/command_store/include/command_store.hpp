#pragma once

#include <string>

namespace command_store
{
    class CommandStore
    {
    public:
        CommandStore();

        void StoreCommand(const std::string& command);
        void DeleteCommand(int id);
    };
} // namespace command_store
