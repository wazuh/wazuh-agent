#include <instance_communicator.hpp>

#include <logger.hpp>

namespace instance_communicator
{
    InstanceCommunicator::InstanceCommunicator(std::function<void()> reloadModulesHandler,
                                               std::function<void(const std::string&)> reloadModuleHandler)
        : m_reloadModulesHandler(std::move(reloadModulesHandler))
        , m_reloadModuleHandler(std::move(reloadModuleHandler))
    {
        if (m_reloadModulesHandler == nullptr || m_reloadModuleHandler == nullptr)
        {
            throw std::runtime_error(std::string("Invalid handler passed."));
        }
    }

    void InstanceCommunicator::HandleSignal(const std::string& signal) const
    {
        if (signal == "RELOAD")
        {
            m_reloadModulesHandler();
        }
        else if (signal.starts_with("RELOAD-MODULE:"))
        {
            m_reloadModuleHandler(signal.substr(signal.find(':') + 1));
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
