#pragma once

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>

namespace instance_communicator
{
    /// @brief Interface for the InstanceCommunicatorWrapper class
    class IInstanceCommunicatorWrapper
    {
    public:
        /// @brief Default destructor
        virtual ~IInstanceCommunicatorWrapper() = default;

        /// @brief Wraps the acceptor open method
        /// @param acceptor The acceptor to open
        virtual void AcceptorOpen(boost::asio::local::stream_protocol::acceptor& acceptor) = 0;

        /// @brief Wraps the acceptor bind method
        /// @param acceptor The acceptor to bind
        /// @param endpoint The endpoint to bind to
        virtual void AcceptorBind(boost::asio::local::stream_protocol::acceptor& acceptor,
                                  const boost::asio::local::stream_protocol::endpoint& endpoint) = 0;

        /// @brief Wraps the acceptor listen method
        /// @param acceptor The acceptor to listen
        virtual void AcceptorListen(boost::asio::local::stream_protocol::acceptor& acceptor) = 0;

        /// @brief Wraps the acceptor close method
        /// @param acceptor The acceptor to close
        virtual void AcceptorClose(boost::asio::local::stream_protocol::acceptor& acceptor) = 0;

        /// @brief Wraps the acceptor async_accept method
        /// @param acceptor The acceptor to async_accept
        /// @param socket The socket to accept on
        /// @param ec The error code from async_accept
        virtual boost::asio::awaitable<void>
        AcceptorAsyncAccept(boost::asio::local::stream_protocol::acceptor& acceptor,
                            boost::asio::local::stream_protocol::socket& socket,
                            boost::system::error_code& ec) = 0;

        /// @brief Wraps the socket read_until method
        /// @param socket The socket to read from
        /// @param buffer The buffer to read into
        /// @param ec The error code from read_until
        virtual boost::asio::awaitable<void> SocketReadUntil(boost::asio::local::stream_protocol::socket& socket,
                                                             boost::asio::streambuf& buffer,
                                                             boost::system::error_code& ec) = 0;
    };
} // namespace instance_communicator
