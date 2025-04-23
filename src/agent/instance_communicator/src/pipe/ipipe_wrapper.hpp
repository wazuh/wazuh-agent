#pragma once

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/system/error_code.hpp>

#include <string>

namespace instance_communicator
{
    class IPipeWrapper
    {
    public:
        /// @brief Default destructor
        virtual ~IPipeWrapper() = default;

        /// @brief Creates the pipe
        /// @param name The name of the pipe
        /// @param bufferSize The size of the buffer
        /// @return True if the pipe was created, false otherwise
        virtual bool PipeCreate(const std::string& name, std::size_t bufferSize) = 0;

        /// @brief Connects to the pipe
        /// @param ec The error code
        /// @return True if the pipe was connected, false otherwise
        virtual boost::asio::awaitable<void> PipeAsyncConnect(boost::system::error_code& ec) = 0;

        /// @brief Reads data from the pipe
        /// @param data The data to read
        /// @param size The size of the data to read
        /// @param ec The error code
        /// @return The number of bytes read
        virtual boost::asio::awaitable<std::size_t>
        PipeAsyncRead(char* data, std::size_t size, boost::system::error_code& ec) = 0;

        /// @brief Closes the pipe
        virtual void PipeClose() = 0;
    };
} // namespace instance_communicator
