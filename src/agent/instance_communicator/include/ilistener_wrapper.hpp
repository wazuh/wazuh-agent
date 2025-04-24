#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/system/error_code.hpp>

#include <string>

namespace instance_communicator
{
    class IListenerWrapper
    {
    public:
        /// @brief Default destructor
        virtual ~IListenerWrapper() = default;

        /// @brief Creates or opens the listener
        /// @param runPath The path to the run directory
        /// @param bufferSize The size of the buffer
        /// @return True if the listener was created or opened, false otherwise
        virtual bool CreateOrOpen(const std::string& runPath, const std::size_t bufferSize = 0) = 0;

        /// @brief Asynchronously accepts a connection (pipe connect or socket accept)
        /// @param ec The error code
        virtual boost::asio::awaitable<void> AsyncAccept(boost::system::error_code& ec) = 0;

        /// @brief Asynchronously reads from the stream
        /// @param data The data to read
        /// @param size The size of the data to read
        /// @param ec The error code
        /// @return The number of bytes read
        virtual boost::asio::awaitable<std::size_t>
        AsyncRead(char* data, const std::size_t size, boost::system::error_code& ec) = 0;

        /// @brief Closes the stream and/or handle
        virtual void Close() = 0;
    };
} // namespace instance_communicator
