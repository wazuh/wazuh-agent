#pragma once

#include <isocket_wrapper.hpp>

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>

#include <string>

namespace instance_communicator
{
    /// @brief Interface for the SocketWrapper class
    class SocketWrapper : public ISocketWrapper
    {
    public:
        /// @brief Constructor
        /// @param io_context The io_context
        explicit SocketWrapper(const boost::asio::any_io_executor& io_context)
            : m_acceptor(io_context)
            , m_socket(io_context)
        {
        }

        /// @copydoc ISocketWrapper::AcceptorOpen
        void AcceptorOpen() override;

        /// @copydoc ISocketWrapper::AcceptorBind
        void AcceptorBind(const boost::asio::local::stream_protocol::endpoint& endpoint) override;

        /// @copydoc ISocketWrapper::AcceptorListen
        void AcceptorListen() override;

        /// @copydoc ISocketWrapper::AcceptorClose
        void AcceptorClose(boost::system::error_code& ec) override;

        /// @copydoc ISocketWrapper::AcceptorAsyncAccept
        boost::asio::awaitable<void> AcceptorAsyncAccept(boost::system::error_code& ec) override;

        /// @copydoc ISocketWrapper::SocketReadUntil
        boost::asio::awaitable<void> SocketReadUntil(boost::asio::streambuf& buffer,
                                                     boost::system::error_code& ec) override;

        /// @copydoc ISocketWrapper::SocketShutdown
        void SocketShutdown(boost::system::error_code& ec) override;

    private:
        /// @brief The acceptor
        boost::asio::local::stream_protocol::acceptor m_acceptor;

        /// @brief The socket
        boost::asio::local::stream_protocol::socket m_socket;
    };
} // namespace instance_communicator
