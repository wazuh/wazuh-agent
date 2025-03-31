#pragma once

#include <iinstance_communicator.hpp>

#include <boost/asio.hpp>

#include <atomic>
#include <functional>
#include <future>
#include <optional>
#include <string>

namespace instance_communicator
{
    constexpr int DEFAULT_PORT = 27001;

    /// @brief Sends signal to the previous instance of the agent
    /// @param message The message to be sent
    /// @param port The port to be used in the conection
    /// @return True if the signal was sent successfully
    bool SendSignal(const std::string& message, uint16_t port = DEFAULT_PORT);

    class InstanceCommunicator : public IInstanceCommunicator
    {
    public:
        /// @brief InstanceCommunicator constructor
        /// @param port The port to be used in the conection
        /// @param handler The handler to be called when a signal is received
        InstanceCommunicator(uint16_t port, std::function<void(const std::string&)> handler);

        /// @copydoc IInstanceCommunicator::Listen
        boost::asio::awaitable<void> Listen(boost::asio::io_context& ioContext) override;

        /// @copydoc IInstanceCommunicator::Stop
        void Stop() override;

    private:
        /// @brief The port to be used in the conection
        uint16_t m_port;

        /// @brief The handler to be called when a signal is received
        std::function<void(const std::string&)> m_signalHandler;

        /// @brief Indicates whether the instance communicator should keep running
        std::atomic<bool> m_keepRunning = true;
    };
} // namespace instance_communicator
