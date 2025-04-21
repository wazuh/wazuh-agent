#include <instance_communicator.hpp>

#include <logger.hpp>

#include <windows.h>

namespace
{
    boost::asio::awaitable<void> AsyncConnectNamedPipe(boost::asio::windows::stream_handle& pipe,
                                                       boost::system::error_code& ec)
    {
        HANDLE native = pipe.native_handle();

        if (native == INVALID_HANDLE_VALUE)
        {
            ec = boost::system::error_code(ERROR_INVALID_HANDLE, boost::system::system_category());
            co_return;
        }

        auto executor = co_await boost::asio::this_coro::executor;
        boost::asio::windows::object_handle eventHandle(executor, CreateEvent(nullptr, TRUE, FALSE, nullptr));

        if (!eventHandle.is_open())
        {
            DWORD err = GetLastError();
            ec = boost::system::error_code(err, boost::system::system_category());
            co_return;
        }

        OVERLAPPED overlapped {};
        overlapped.hEvent = eventHandle.native_handle();

        BOOL result = ConnectNamedPipe(native, &overlapped);
        if (result)
        {
            ec.clear();
            co_return;
        }

        DWORD lastError = GetLastError();
        if (lastError == ERROR_PIPE_CONNECTED)
        {
            ec.clear();
            co_return;
        }
        else if (lastError != ERROR_IO_PENDING)
        {
            ec = boost::system::error_code(lastError, boost::system::system_category());
            co_return;
        }

        try
        {
            co_await eventHandle.async_wait(boost::asio::use_awaitable);

            DWORD bytesTransferred;
            if (GetOverlappedResult(native, &overlapped, &bytesTransferred, FALSE))
            {
                ec.clear();
            }
            else
            {
                DWORD err = GetLastError();
                ec = boost::system::error_code(err, boost::system::system_category());
            }
        }
        catch (const boost::system::system_error& e)
        {
            ec = e.code();
        }

        co_return;
    }

} // namespace

namespace instance_communicator
{
    boost::asio::awaitable<void>
    InstanceCommunicator::Listen([[maybe_unused]] const std::string&
                                     socketFilePath, // NOLINT (cppcoreguidelines-avoid-reference-coroutine-parameters)
                                 [[maybe_unused]] std::unique_ptr<ISocketWrapper> socketWrapper)
    {
        constexpr int A_SECOND_IN_MILLIS = 1000;
        constexpr int BUFFER_SIZE = 4096;
        auto executor = co_await boost::asio::this_coro::executor;

        const std::string pipeName = "\\\\.\\pipe\\agent-pipe";

        while (m_keepRunning.load())
        {
            HANDLE pipeHandle = CreateNamedPipe(pipeName.c_str(),
                                                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                                PIPE_UNLIMITED_INSTANCES,
                                                BUFFER_SIZE, // Output buffer size
                                                BUFFER_SIZE, // Input buffer size
                                                0,           // Default timeout
                                                nullptr);    // Default security

            if (pipeHandle == INVALID_HANDLE_VALUE)
            {
                DWORD err = GetLastError();
                LogError("CreateNamedPipe failed (code {}): {}", err, std::system_category().message(err));

                boost::asio::steady_timer timer(executor, std::chrono::milliseconds(A_SECOND_IN_MILLIS));
                co_await timer.async_wait(boost::asio::use_awaitable);

                continue;
            }

            LogDebug("InstanceCommunicator listening on {}", pipeName);

            boost::asio::windows::stream_handle pipeStream(executor, pipeHandle);

            boost::system::error_code ec;
            try
            {
                co_await AsyncConnectNamedPipe(pipeStream, ec);

                if (ec)
                {
                    LogError("Named pipe connection failed (code {}): {}", ec.value(), ec.message());
                }
                else if (!pipeStream.is_open())
                {
                    LogError("Pipe stream is not open after connection.");
                }
                else
                {
                    std::array<char, BUFFER_SIZE> buffer {};
                    std::size_t n =
                        co_await pipeStream.async_read_some(boost::asio::buffer(buffer), boost::asio::use_awaitable);

                    std::string message(buffer.data(), n);

                    if (auto pos = message.find('\n'); pos != std::string::npos)
                    {
                        message.erase(pos, 1);
                    }

                    LogDebug("Received signal: {}", message);

                    HandleSignal(message);
                }
            }
            catch (const std::exception& e)
            {
                LogError("Listener exception: {}", e.what());
            }

            if (pipeStream.is_open())
            {
                pipeStream.close();
            }

            CloseHandle(pipeHandle);

            boost::asio::steady_timer timer(executor, std::chrono::milliseconds(A_SECOND_IN_MILLIS));
            co_await timer.async_wait(boost::asio::use_awaitable);
        }

        LogDebug("InstanceCommunicator stopping");
        co_return;
    }
} // namespace instance_communicator
