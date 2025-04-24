#include <ipipe_wrapper.hpp>
#include <listener_wrapper.hpp>
#include <pipe_wrapper.hpp>

#include <boost/asio.hpp>

#include <memory>
#include <string>

namespace instance_communicator
{
    const std::string ENDPOINT = "\\\\.\\pipe\\agent-pipe";

    template<>
    ListenerWrapper<IPipeWrapper>::ListenerWrapper(const boost::asio::any_io_executor& executor)
        : m_listener(std::make_unique<PipeWrapper>(executor))
    {
    }

    template<>
    bool ListenerWrapper<IPipeWrapper>::CreateOrOpen([[maybe_unused]] const std::string& runPath,
                                                     const std::size_t bufferSize)
    {
        return m_listener->PipeCreate(ENDPOINT, bufferSize);
    }

    template<>
    boost::asio::awaitable<void> ListenerWrapper<IPipeWrapper>::AsyncAccept(
        boost::system::error_code& ec) // NOLINT(cppcoreguidelines-avoid-reference-coroutine-parameters)
    {
        co_await m_listener->PipeAsyncConnect(ec);
    }

    template<>
    boost::asio::awaitable<std::size_t> ListenerWrapper<IPipeWrapper>::AsyncRead(
        char* data,
        const std::size_t size,
        boost::system::error_code& ec) // NOLINT(cppcoreguidelines-avoid-reference-coroutine-parameters)
    {
        co_return co_await m_listener->PipeAsyncRead(data, size, ec);
    }

    template<>
    void ListenerWrapper<IPipeWrapper>::Close()
    {
        m_listener->PipeClose();
    }
} // namespace instance_communicator
