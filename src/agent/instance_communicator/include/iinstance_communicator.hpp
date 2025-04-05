#pragma once

#include <boost/asio/awaitable.hpp>

namespace instance_communicator
{
    /// @brief Interface for the InstanceCommunicator class
    class IInstanceCommunicator
    {
    public:
        /// @brief Default destructor
        virtual ~IInstanceCommunicator() = default;

        /// @brief Starts listening for incoming connections
        /// @param ioContext The io context to be used
        virtual boost::asio::awaitable<void> Listen(boost::asio::io_context& ioContext) = 0;

        /// @brief Stops the Instance Communicator
        virtual void Stop() = 0;
    };
} // namespace instance_communicator
