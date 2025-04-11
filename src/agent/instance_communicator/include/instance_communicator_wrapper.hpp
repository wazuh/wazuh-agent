#pragma once

#include <iinstance_communicator_wrapper.hpp>

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>

namespace instance_communicator
{
    /// @brief Interface for the InstanceCommunicatorWrapper class
    class InstanceCommunicatorWrapper : public IInstanceCommunicatorWrapper
    {
    public:
        /// @copydoc IInstanceCommunicatorWrapper::AcceptorOpen
        void AcceptorOpen(boost::asio::local::stream_protocol::acceptor& acceptor) override;

        /// @copydoc IInstanceCommunicatorWrapper::AcceptorBind
        void AcceptorBind(boost::asio::local::stream_protocol::acceptor& acceptor,
                          const boost::asio::local::stream_protocol::endpoint& endpoint) override;

        /// @copydoc IInstanceCommunicatorWrapper::AcceptorListen
        void AcceptorListen(boost::asio::local::stream_protocol::acceptor& acceptor) override;

        /// @copydoc IInstanceCommunicatorWrapper::AcceptorClose
        void AcceptorClose(boost::asio::local::stream_protocol::acceptor& acceptor) override;

        /// @copydoc IInstanceCommunicatorWrapper::AcceptorAsyncAccept
        boost::asio::awaitable<void> AcceptorAsyncAccept(boost::asio::local::stream_protocol::acceptor& acceptor,
                                                         boost::asio::local::stream_protocol::socket& socket,
                                                         boost::system::error_code& ec) override;

        /// @copydoc IInstanceCommunicatorWrapper::SocketReadUntil
        boost::asio::awaitable<void> SocketReadUntil(boost::asio::local::stream_protocol::socket& socket,
                                                     boost::asio::streambuf& buffer,
                                                     boost::system::error_code& ec) override;
    };
} // namespace instance_communicator
