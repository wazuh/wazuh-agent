#pragma once

#include <iinstance_communicator_wrapper.hpp>
#include <instance_communicator_wrapper.hpp>

#include <boost/asio/awaitable.hpp>

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
        /// @param ioContext The io context to be used
        /// @param wrapper The wrapper to be used for mocking in unit tests (nullptr in production)
        virtual boost::asio::awaitable<void>
        Listen(boost::asio::io_context& ioContext, std::unique_ptr<IInstanceCommunicatorWrapper> wrapper = nullptr) = 0;

        /// @brief Stops the Instance Communicator
        virtual void Stop() = 0;
    };
} // namespace instance_communicator
