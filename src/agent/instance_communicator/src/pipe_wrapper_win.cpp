#include <pipe_wrapper_win.hpp>

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>

#include <windows.h>

namespace instance_communicator
{
    bool PipeWrapper::PipeCreate(const std::string& name, std::size_t bufferSize)
    {
        m_pipeHandle = CreateNamedPipeA(name.c_str(),
                                        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                        PIPE_UNLIMITED_INSTANCES,
                                        static_cast<DWORD>(bufferSize),
                                        static_cast<DWORD>(bufferSize),
                                        0,
                                        nullptr);

        if (m_pipeHandle == nullptr || m_pipeHandle == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        boost::system::error_code ec;
        m_pipeStream.assign(m_pipeHandle, ec);

        if (ec || !m_pipeStream.is_open())
        {
            CloseHandle(m_pipeHandle);
            m_pipeHandle = INVALID_HANDLE_VALUE;
            return false;
        }

        return true;
    }

    boost::asio::awaitable<void> PipeWrapper::PipeAsyncConnect(boost::system::error_code& ec)
    {
        if (m_pipeHandle == nullptr || m_pipeHandle == INVALID_HANDLE_VALUE)
        {
            ec = boost::system::error_code(ERROR_BROKEN_PIPE, boost::system::system_category());
            co_return;
        }

        auto executor = co_await boost::asio::this_coro::executor;
        HANDLE event = CreateEvent(nullptr, TRUE, FALSE, nullptr);

        if (event == nullptr || event == INVALID_HANDLE_VALUE)
        {
            DWORD err = GetLastError();
            ec = boost::system::error_code(err, boost::system::system_category());
            co_return;
        }

        boost::asio::windows::object_handle eventHandle(executor, event);

        OVERLAPPED overlapped {};
        overlapped.hEvent = event;

        BOOL result = ConnectNamedPipe(m_pipeHandle, &overlapped);
        DWORD lastError = GetLastError();

        if (result || lastError == ERROR_PIPE_CONNECTED)
        {
            ec.clear();
            co_return;
        }

        if (lastError != ERROR_IO_PENDING)
        {
            ec = boost::system::error_code(lastError, boost::system::system_category());
            co_return;
        }

        try
        {
            co_await eventHandle.async_wait(boost::asio::use_awaitable);

            DWORD transferred = 0;
            if (GetOverlappedResult(m_pipeHandle, &overlapped, &transferred, FALSE))
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

    boost::asio::awaitable<std::size_t>
    PipeWrapper::PipeAsyncRead(char* data, std::size_t size, boost::system::error_code& ec)
    {
        if (!m_pipeStream.is_open())
        {
            ec = boost::system::error_code(ERROR_BROKEN_PIPE, boost::system::system_category());
            co_return 0;
        }

        co_return co_await m_pipeStream.async_read_some(boost::asio::buffer(data, size),
                                                        boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    }

    void PipeWrapper::PipeClose()
    {
        if (m_pipeStream.is_open())
        {
            boost::system::error_code ec;
            m_pipeStream.close(ec);
        }

        if (m_pipeHandle != nullptr && m_pipeHandle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_pipeHandle);
            m_pipeHandle = INVALID_HANDLE_VALUE;
        }
    }
} // namespace instance_communicator
