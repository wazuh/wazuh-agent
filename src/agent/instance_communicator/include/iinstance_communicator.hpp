#pragma once

#include <ipipe_wrapper.hpp>
#include <isocket_wrapper.hpp>

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
        /// @param runPath The path where to listen
        /// @param socketWrapper The wrapper to be used for mocking socket functions
        /// @param pipeWrapper The wrapper to be used for mocking pipe functions
        virtual boost::asio::awaitable<void> Listen(const std::string& runPath,
                                                    std::unique_ptr<ISocketWrapper> socketWrapper = nullptr,
                                                    std::unique_ptr<IPipeWrapper> pipeWrapper = nullptr) = 0;

        /// @brief Stops the Instance Communicator
        virtual void Stop() = 0;
    };
} // namespace instance_communicator
