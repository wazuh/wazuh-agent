#pragma once

#include <ilistener_wrapper.hpp>
#include <ipipe_wrapper.hpp>
#include <pipe_wrapper.hpp>

#include <boost/asio.hpp>

#include <memory>
#include <string>

namespace instance_communicator
{
    class PipeListenerWrapper : public IListenerWrapper
    {
    public:
        explicit PipeListenerWrapper(boost::asio::any_io_executor executor)
            : m_pipe(std::make_unique<PipeWrapper>(executor))
        {
        }

        bool CreateOrOpen(const std::string& name, std::size_t bufferSize) override
        {
            return m_pipe->PipeCreate(name, bufferSize);
        }

        boost::asio::awaitable<void> AsyncAccept(boost::system::error_code& ec) override
        {
            co_await m_pipe->PipeAsyncConnect(ec);
        }

        boost::asio::awaitable<std::size_t>
        AsyncRead(char* data, std::size_t size, boost::system::error_code& ec) override
        {
            co_return co_await m_pipe->PipeAsyncRead(data, size, ec);
        }

        void Close() override
        {
            m_pipe->PipeClose();
        }

    private:
        std::unique_ptr<IPipeWrapper> m_pipe;
    };
} // namespace instance_communicator
