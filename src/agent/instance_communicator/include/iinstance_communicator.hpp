#pragma once

#include <ilistener_wrapper.hpp>

#include <boost/asio/awaitable.hpp>

#include <string>

namespace instance_communicator
{
    /// @brief Interface for the InstanceCommunicator class
    class IInstanceCommunicator
    {
    public:
        /// @brief Default destructor
        virtual ~IInstanceCommunicator() = default;

        /// @brief Handles signals sent by other instance of the Agent
        /// @param signal received
        virtual void HandleSignal(const std::string& signal) const = 0;

        /// @brief Starts listening for incoming connections
        /// @param runPath The path to the run directory
        /// @param listenerWrapper The listener wrapper to use
        virtual boost::asio::awaitable<void> Listen(const std::string& runPath,
                                                    std::unique_ptr<IListenerWrapper> listenerWrapper = nullptr) = 0;

        /// @brief Stops the Instance Communicator
        virtual void Stop() = 0;
    };
} // namespace instance_communicator
