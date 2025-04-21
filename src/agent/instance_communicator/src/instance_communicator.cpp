#include <instance_communicator.hpp>

#include <logger.hpp>

namespace instance_communicator
{
    InstanceCommunicator::InstanceCommunicator(
        std::function<void(const std::optional<std::string>&)> reloadModulesHandler)
        : m_reloadModulesHandler(std::move(reloadModulesHandler))
    {
        if (m_reloadModulesHandler == nullptr)
        {
            throw std::runtime_error(std::string("Invalid handler passed."));
        }
    }

    void InstanceCommunicator::HandleSignal(const std::string& signal) const
    {
        if (signal == "RELOAD")
        {
            m_reloadModulesHandler(std::nullopt);
        }
        else if (signal.starts_with("RELOAD-MODULE:"))
        {
            m_reloadModulesHandler(signal.substr(signal.find(':') + 1));
        }
        else
        {
            LogWarn("Invalid message received from CLI: {}", signal);
        }
    }

    void InstanceCommunicator::Stop()
    {
        m_keepRunning.store(false);
    }
} // namespace instance_communicator
