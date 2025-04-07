#include <instance_communicator.hpp>

#include <logger.hpp>

#include <windows.h>

#include <iostream>

namespace instance_communicator
{
    boost::asio::awaitable<void> AsyncConnectNamedPipe(boost::asio::windows::stream_handle& pipe,
                                                       boost::system::error_code& ec)
    {
        HANDLE native = pipe.native_handle();
        BOOL result = ConnectNamedPipe(native, nullptr);

        if (!result && GetLastError() != ERROR_PIPE_CONNECTED)
        {
            ec = boost::system::error_code(GetLastError(), boost::system::system_category());
            co_return;
        }

        ec = {};
        co_return;
    }

    boost::asio::awaitable<void> InstanceCommunicator::Listen(
        boost::asio::io_context& ioContext) // NOLINT (cppcoreguidelines-avoid-reference-coroutine-parameters)
    {
        const std::string pipeName = "\\\\.\\pipe\\agent-pipe";

        while (m_keepRunning.load())
        {
            try
            {
                // Create named pipe
                HANDLE pipeHandle = CreateNamedPipe(pipeName.c_str(),
                                                    PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                                    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                                    PIPE_UNLIMITED_INSTANCES,
                                                    4096,  // Output buffer size
                                                    4096,  // Input buffer size
                                                    0,     // Default timeout
                                                    NULL); // Default security

                if (pipeHandle == INVALID_HANDLE_VALUE)
                {
                    LogError("CreateNamedPipe failed: {}", GetLastError());
                    co_await boost::asio::steady_timer(ioContext, std::chrono::seconds(1))
                        .async_wait(boost::asio::use_awaitable);

                    continue;
                }

                LogDebug("InstanceCommunicator listening on {}", pipeName);

                boost::asio::windows::stream_handle pipeStream(ioContext, pipeHandle);

                boost::system::error_code ec;
                co_await AsyncConnectNamedPipe(pipeStream, ec);

                if (!ec)
                {
                    std::array<char, 1024> buffer {};
                    std::size_t n =
                        co_await pipeStream.async_read_some(boost::asio::buffer(buffer), boost::asio::use_awaitable);

                    std::string message(buffer.data(), n);
                    message.erase(std::remove(message.begin(), message.end(), '\n'), message.end());

                    LogWarn("Server received signal: {}", message);

                    HandleSignal(message);
                }
                else
                {
                    LogError("Named pipe connection failed: {}", ec.message());
                }

                pipeStream.close();

                CloseHandle(pipeHandle);
            }
            catch (const std::exception& e)
            {
                LogError("Listener exception: {}", e.what());
            }

            co_await boost::asio::steady_timer(ioContext, std::chrono::seconds(1))
                .async_wait(boost::asio::use_awaitable);
        }

        LogDebug("InstanceCommunicator stopping");
        co_return;
    }
} // namespace instance_communicator
