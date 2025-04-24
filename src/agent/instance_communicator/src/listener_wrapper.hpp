#pragma once

#include <ilistener_wrapper.hpp>

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/system/error_code.hpp>

#include <memory>
#include <string>

namespace instance_communicator
{
    template<class ListenerType>
    class ListenerWrapper : public IListenerWrapper
    {
    public:
        explicit ListenerWrapper(const boost::asio::any_io_executor& executor);

        bool CreateOrOpen([[maybe_unused]] const std::string& runPath, const std::size_t bufferSize) override;

        boost::asio::awaitable<void> AsyncAccept(boost::system::error_code& ec) override;

        boost::asio::awaitable<std::size_t>
        AsyncRead(char* data, const std::size_t size, boost::system::error_code& ec) override;

        void Close() override;

    private:
        std::unique_ptr<ListenerType> m_listener;
    };
} // namespace instance_communicator
