#pragma once

#include <iinstance_communicator.hpp>

#include <boost/asio.hpp>

#include <atomic>
#include <functional>
#include <string>

namespace instance_communicator
{
    /// @brief The path/name of the socket/named pipe
    const std::string socketPath = "/tmp/agent-socket";

    /// @brief Sends signal to the previous instance of the agent
    /// @param message The message to be sent
    /// @return True if the signal was sent successfully
    bool SendSignal(const std::string& message);

    class InstanceCommunicator : public IInstanceCommunicator
    {
    public:
        /// @brief InstanceCommunicator constructor
        /// @param handler The handler to be called when a signal is received
        InstanceCommunicator(std::function<void(const std::string&)> handler);

        /// @copydoc IInstanceCommunicator::Listen
        boost::asio::awaitable<void> Listen(boost::asio::io_context& ioContext) override;

        /// @copydoc IInstanceCommunicator::Stop
        void Stop() override;

    private:
        /// @brief The handler to be called when a signal is received
        std::function<void(const std::string&)> m_signalHandler;

        /// @brief Indicates whether the instance communicator should keep running
        std::atomic<bool> m_keepRunning = true;
    };
} // namespace instance_communicator
