#include <isocket_wrapper.hpp>
#include <listener_wrapper.hpp>
#include <socket_wrapper.hpp>

#include <boost/asio.hpp>

#include <fmt/format.h>
#include <istream>
#include <memory>
#include <string>

namespace instance_communicator
{
    const std::string ENDPOINT = "agent-socket";

    template<>
    ListenerWrapper<ISocketWrapper>::ListenerWrapper(const boost::asio::any_io_executor& executor)
        : m_listener(std::make_unique<SocketWrapper>(executor))
    {
    }

    template<>
    bool ListenerWrapper<ISocketWrapper>::CreateOrOpen(const std::string& runPath,
                                                       [[maybe_unused]] std::size_t bufferSize)
    {
        constexpr mode_t SOCKET_FILE_PERMISSIONS = 0660;

        const std::string name = fmt::format("{}/{}", runPath, ENDPOINT);

        try
        {
            ::unlink(name.c_str());

            using boost::asio::local::stream_protocol;
            const stream_protocol::endpoint endpoint(name);

            m_listener->AcceptorOpen();
            m_listener->AcceptorBind(endpoint);
            m_listener->AcceptorListen();

            ::chmod(name.c_str(), SOCKET_FILE_PERMISSIONS);

            return true;
        }
        catch (...) // NOLINT(bugprone-empty-catch)
        {
            return false;
        }
    }

    template<>
    boost::asio::awaitable<void> ListenerWrapper<ISocketWrapper>::AsyncAccept(
        boost::system::error_code& ec) // NOLINT(cppcoreguidelines-avoid-reference-coroutine-parameters)
    {
        co_await m_listener->AcceptorAsyncAccept(ec);
    }

    template<>
    boost::asio::awaitable<std::size_t> ListenerWrapper<ISocketWrapper>::AsyncRead(
        char* data,
        std::size_t size,
        boost::system::error_code& ec) // NOLINT(cppcoreguidelines-avoid-reference-coroutine-parameters)
    {
        auto buf = std::make_shared<boost::asio::streambuf>();
        co_await m_listener->SocketReadUntil(*buf, ec);

        std::istream is(buf.get());
        is.read(data, static_cast<std::streamsize>(size));

        co_return static_cast<std::size_t>(is.gcount());
    }

    template<>
    void ListenerWrapper<ISocketWrapper>::Close()
    {
        boost::system::error_code ec;
        m_listener->SocketShutdown(ec);
        m_listener->AcceptorClose();
    }
} // namespace instance_communicator
