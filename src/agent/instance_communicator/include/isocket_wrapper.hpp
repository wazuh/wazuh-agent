#pragma once

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>

namespace instance_communicator
{
    /// @brief Interface for the ISocketWrapper class
    class ISocketWrapper
    {
    public:
        /// @brief Default destructor
        virtual ~ISocketWrapper() = default;

        /// @brief Wraps the acceptor open method
        virtual void AcceptorOpen() = 0;

        /// @brief Wraps the acceptor bind method
        /// @param endpoint The endpoint to bind to
        virtual void AcceptorBind(const boost::asio::local::stream_protocol::endpoint& endpoint) = 0;

        /// @brief Wraps the acceptor listen method
        virtual void AcceptorListen() = 0;

        /// @brief Wraps the acceptor close method
        virtual void AcceptorClose() = 0;

        /// @brief Wraps the acceptor async_accept method
        /// @param ec The error code from async_accept
        virtual boost::asio::awaitable<void> AcceptorAsyncAccept(boost::system::error_code& ec) = 0;

        /// @brief Wraps the socket read_until method
        /// @param buffer The buffer to read into
        /// @param ec The error code from read_until
        virtual boost::asio::awaitable<void> SocketReadUntil(boost::asio::streambuf& buffer,
                                                             boost::system::error_code& ec) = 0;

        /// @brief Wraps the socket shutdown method
        /// @param ec The error code from shutdown
        virtual void SocketShutdown(boost::system::error_code& ec) = 0;
    };
} // namespace instance_communicator
