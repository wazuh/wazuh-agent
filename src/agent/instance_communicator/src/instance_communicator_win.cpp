#include <instance_communicator.hpp>

#include <logger.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include <windows.h>

#include <iostream>

namespace instance_communicator
{
    const std::string pipeName = "\\\\.\\pipe\\agent-pipe";

    bool SendSignal(const std::string& message)
    {
        HANDLE hPipe = CreateFile(pipeName.c_str(), // pipe name
                                  GENERIC_WRITE,    // write access
                                  0,                // no sharing
                                  NULL,             // default security attributes
                                  OPEN_EXISTING,    // opens existing pipe
                                  0,                // default attributes
                                  NULL);            // no template file

        if (hPipe == INVALID_HANDLE_VALUE)
        {
            std::cerr << "Client connect error: " << GetLastError() << "\n";
            return false;
        }

        std::string command = message + "\n";
        DWORD bytesWritten = 0;

        BOOL success = WriteFile(hPipe,                              // pipe handle
                                 command.c_str(),                    // message
                                 static_cast<DWORD>(command.size()), // message length
                                 &bytesWritten,                      // bytes written
                                 NULL);                              // not overlapped

        if (!success || bytesWritten != command.size())
        {
            std::cerr << "Client write error: " << GetLastError() << "\n";
            CloseHandle(hPipe);
            return false;
        }

        std::cout << "Client sent signal: " << message << "\n";

        // Done writing: close the handle to signal EOF
        CloseHandle(hPipe);
        return true;
    }

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
        // LogWarn(L"Setting InstanceCommunicator to listen on named pipe {}", pipeName.c_str());

        while (m_keepRunning.load())
        {
            bool delayRetry = true;

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

                    if (m_signalHandler && !message.empty())
                    {
                        m_signalHandler(message);
                    }
                    delayRetry = false;
                }
                else
                {
                    LogError("Named pipe connection failed: {}", ec.message());
                }

                pipeStream.close();
            }
            catch (const std::exception& e)
            {
                LogError("Listener exception: {}", e.what());
            }

            if (delayRetry)
            {
                co_await boost::asio::steady_timer(ioContext, std::chrono::seconds(1))
                    .async_wait(boost::asio::use_awaitable);
            }
        }

        LogDebug("InstanceCommunicator stopping");
        co_return;
    }
} // namespace instance_communicator
