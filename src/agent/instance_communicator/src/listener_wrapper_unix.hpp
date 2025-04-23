#pragma once

#include <ilistener_wrapper.hpp>
#include <isocket_wrapper.hpp>
#include <socket_wrapper.hpp>

#include <boost/asio.hpp>

#include <istream>
#include <memory>
#include <string>

namespace instance_communicator
{
    class SocketListenerWrapper : public IListenerWrapper
    {
    public:
        explicit SocketListenerWrapper(boost::asio::any_io_executor executor)
            : m_socket(std::make_unique<SocketWrapper>(executor))
        {
        }

        bool CreateOrOpen(const std::string& name, [[maybe_unused]] std::size_t bufferSize) override
        {
            ::unlink(name.c_str());

            try
            {
                using boost::asio::local::stream_protocol;
                stream_protocol::endpoint endpoint(name);

                m_socket->AcceptorOpen();
                m_socket->AcceptorBind(endpoint);
                m_socket->AcceptorListen();

                return true;
            }
            catch (...) // NOLINT(bugprone-empty-catch)
            {
                return false;
            }
        }

        boost::asio::awaitable<void> AsyncAccept(boost::system::error_code& ec) override
        {
            co_await m_socket->AcceptorAsyncAccept(ec);
        }

        boost::asio::awaitable<std::size_t>
        AsyncRead(char* data, std::size_t size, boost::system::error_code& ec) override
        {
            auto buf = std::make_shared<boost::asio::streambuf>();
            co_await m_socket->SocketReadUntil(*buf, ec);

            std::istream is(buf.get());
            is.read(data, static_cast<std::streamsize>(size));

            co_return static_cast<std::size_t>(is.gcount());
        }

        void Close() override
        {
            boost::system::error_code ec;
            m_socket->SocketShutdown(ec);
            m_socket->AcceptorClose();
        }

    private:
        std::unique_ptr<ISocketWrapper> m_socket;
    };
} // namespace instance_communicator
