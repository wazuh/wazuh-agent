#pragma once

#include <iinstance_communicator.hpp>
#include <ilistener_wrapper.hpp>

#include <boost/asio/awaitable.hpp>

#include <atomic>
#include <functional>
#include <optional>
#include <string>

namespace instance_communicator
{
    class InstanceCommunicator : public IInstanceCommunicator
    {
    public:
        /// @brief InstanceCommunicator constructor
        /// @param reloadModulesHandler The handler to be called when a signal to reload modules is received
        InstanceCommunicator(std::function<void(const std::optional<std::string>&)> reloadModulesHandler);

        /// @copydoc IInstanceCommunicator::HandleSignal
        void HandleSignal(const std::string& signal) const override;

        /// @copydoc IInstanceCommunicator::Listen
        boost::asio::awaitable<void> Listen(const std::string& endpointName,
                                            std::unique_ptr<IListenerWrapper> listenerWrapper = nullptr) override;

        /// @copydoc IInstanceCommunicator::Stop
        void Stop() override;

    private:
        /// @brief The handler to be called when a signal to reload modules is received
        std::function<void(const std::optional<std::string>&)> m_reloadModulesHandler;

        /// @brief Indicates whether the instance communicator should keep running
        std::atomic<bool> m_keepRunning = true;
    };
} // namespace instance_communicator
