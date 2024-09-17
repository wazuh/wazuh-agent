#include <command_handler.hpp>

namespace command_handler
{
    void CommandHandler::Stop()
    {
        m_keepRunning.store(false);
    }
} // namespace command_handler
