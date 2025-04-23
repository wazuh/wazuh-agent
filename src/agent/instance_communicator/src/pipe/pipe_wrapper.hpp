#pragma once

#include <ipipe_wrapper.hpp>

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/windows/stream_handle.hpp>

#include <string>

namespace instance_communicator
{
    class PipeWrapper : public IPipeWrapper
    {
    public:
        /// @brief Constructor
        /// @param executor The executor
        explicit PipeWrapper(boost::asio::any_io_executor executor)
            : m_pipeStream {executor}
        {
        }

        /// @copydoc IPipeWrapper::PipeCreate
        bool PipeCreate(const std::string& name, std::size_t bufferSize) override;

        /// @copydoc IPipeWrapper::PipeAsyncConnect
        boost::asio::awaitable<void> PipeAsyncConnect(boost::system::error_code& ec) override;

        /// @copydoc IPipeWrapper::PipeAsyncRead
        boost::asio::awaitable<std::size_t>
        PipeAsyncRead(char* data, std::size_t size, boost::system::error_code& ec) override;

        /// @copydoc IPipeWrapper::PipeClose
        void PipeClose() override;

    private:
        /// @brief The pipe stream
        boost::asio::windows::stream_handle m_pipeStream;

        /// @brief The pipe handle
        HANDLE m_pipeHandle = INVALID_HANDLE_VALUE;
    };
} // namespace instance_communicator
