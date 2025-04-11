
#include <instance_communicator_wrapper.hpp>

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>

namespace instance_communicator
{
    void InstanceCommunicatorWrapper::AcceptorOpen(boost::asio::local::stream_protocol::acceptor& acceptor)
    {
        acceptor.open();
    }

    void InstanceCommunicatorWrapper::AcceptorBind(boost::asio::local::stream_protocol::acceptor& acceptor,
                                                   const boost::asio::local::stream_protocol::endpoint& endpoint)
    {
        acceptor.bind(endpoint);
    }

    void InstanceCommunicatorWrapper::AcceptorListen(boost::asio::local::stream_protocol::acceptor& acceptor)
    {
        acceptor.listen();
    }

    void InstanceCommunicatorWrapper::AcceptorClose(boost::asio::local::stream_protocol::acceptor& acceptor)
    {
        acceptor.close();
    }

    boost::asio::awaitable<void>
    InstanceCommunicatorWrapper::AcceptorAsyncAccept(boost::asio::local::stream_protocol::acceptor& acceptor, // NOLINT
                                                     boost::asio::local::stream_protocol::socket& socket,     // NOLINT
                                                     boost::system::error_code& ec)                           // NOLINT
    {
        co_return co_await acceptor.async_accept(socket, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    }

    boost::asio::awaitable<void>
    InstanceCommunicatorWrapper::SocketReadUntil(boost::asio::local::stream_protocol::socket& socket, // NOLINT
                                                 boost::asio::streambuf& buffer,                      // NOLINT
                                                 boost::system::error_code& ec)                       // NOLINT
    {
        co_await boost::asio::async_read_until(
            socket, buffer, "\n", boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        co_return;
    }

} // namespace instance_communicator
