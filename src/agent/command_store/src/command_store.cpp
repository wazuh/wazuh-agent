#include <command_store.hpp>

#include <iostream>

namespace command_store
{
    CommandStore::CommandStore()
    {
        std::cout << "CommandStore constructor\n";
        std::cout << "Create SqliteManager object\n";
        std::cout << "Create table command\n";
    }

    void CommandStore::StoreCommand(const std::string& command)
    {
        std::cout << "Store command: " << command << "\n";
    }

    void CommandStore::DeleteCommand(int id)
    {
        std::cout << "Deleting command " << id << "\n";
    }

} // namespace command_store
