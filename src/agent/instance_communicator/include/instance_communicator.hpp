#pragma once

#include <iinstance_communicator.hpp>

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>

#include <atomic>
#include <functional>
#include <string>

namespace instance_communicator
{
    class InstanceCommunicator : public IInstanceCommunicator
    {
    public:
        /// @brief InstanceCommunicator constructor
        /// @param reloadModulesHandler The handler to be called when a RELOAD signal is received
        /// @param reloadModuleHandler The handler to be called when a RELOAD-MODULE signal is received
        InstanceCommunicator(std::function<void()> reloadModulesHandler,
                             std::function<void(const std::string&)> reloadModuleHandler);

        /// @copydoc IInstanceCommunicator::HandleSignal
        void HandleSignal(const std::string& signal) const override;

        /// @copydoc IInstanceCommunicator::Listen
        boost::asio::awaitable<void> Listen(boost::asio::io_context& ioContext,
                                            std::unique_ptr<IInstanceCommunicatorWrapper> wrapper = nullptr) override;

        /// @copydoc IInstanceCommunicator::Stop
        void Stop() override;

    private:
        /// @brief The handler to be called when a RELOAD signal is received
        std::function<void()> m_reloadModulesHandler;

        /// @brief The handler to be called when a RELOAD-MODULES signal is received
        std::function<void(const std::string&)> m_reloadModuleHandler;

        /// @brief Indicates whether the instance communicator should keep running
        std::atomic<bool> m_keepRunning = true;
    };
} // namespace instance_communicator
