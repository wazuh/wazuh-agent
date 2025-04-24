
#include <socket_wrapper.hpp>

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>

namespace instance_communicator
{
    void SocketWrapper::AcceptorOpen()
    {
        m_acceptor.open();
    }

    void SocketWrapper::AcceptorBind(const boost::asio::local::stream_protocol::endpoint& endpoint)
    {
        m_acceptor.bind(endpoint);
    }

    void SocketWrapper::AcceptorListen()
    {
        m_acceptor.listen();
    }

    void SocketWrapper::AcceptorClose(boost::system::error_code& ec)
    {
        m_acceptor.close(ec); // NOLINT(bugprone-unused-return-value)
    }

    // NOLINTBEGIN(cppcoreguidelines-avoid-reference-coroutine-parameters)
    boost::asio::awaitable<void> SocketWrapper::AcceptorAsyncAccept(boost::system::error_code& ec)
    {
        co_await m_acceptor.async_accept(m_socket, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    }

    boost::asio::awaitable<void> SocketWrapper::SocketReadUntil(boost::asio::streambuf& buffer,
                                                                boost::system::error_code& ec)
    {
        co_await boost::asio::async_read_until(
            m_socket, buffer, '\n', boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    }

    void SocketWrapper::SocketShutdown(boost::system::error_code& ec)
    {
        // NOLINTBEGIN(bugprone-unused-return-value)
        m_socket.shutdown(boost::asio::socket_base::shutdown_type::shutdown_both, ec);
        m_socket.close(ec);
        // NOLINTEND(bugprone-unused-return-value)
    }

    // NOLINTEND(cppcoreguidelines-avoid-reference-coroutine-parameters)

} // namespace instance_communicator
