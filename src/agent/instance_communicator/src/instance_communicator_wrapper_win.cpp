
#include <instance_communicator_wrapper.hpp>

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>

namespace instance_communicator
{
    void InstanceCommunicatorWrapper::AcceptorOpen(boost::asio::local::stream_protocol::acceptor&) {}

    void InstanceCommunicatorWrapper::AcceptorBind(boost::asio::local::stream_protocol::acceptor&,
                                                   const boost::asio::local::stream_protocol::endpoint&)
    {
    }

    void InstanceCommunicatorWrapper::AcceptorListen(boost::asio::local::stream_protocol::acceptor&) {}

    void InstanceCommunicatorWrapper::AcceptorClose(boost::asio::local::stream_protocol::acceptor&) {}

    boost::asio::awaitable<void>
    InstanceCommunicatorWrapper::AcceptorAsyncAccept(boost::asio::local::stream_protocol::acceptor&, // NOLINT
                                                     boost::asio::local::stream_protocol::socket&,   // NOLINT
                                                     boost::system::error_code&)                     // NOLINT
    {
        co_return;
    }

    boost::asio::awaitable<void>
    InstanceCommunicatorWrapper::SocketReadUntil(boost::asio::local::stream_protocol::socket&, // NOLINT
                                                 boost::asio::streambuf&,                      // NOLINT
                                                 boost::system::error_code&)                   // NOLINT
    {
        co_return;
    }

} // namespace instance_communicator
