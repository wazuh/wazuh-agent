#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <optional>
#include <string>

#include <boost/asio.hpp>

namespace instance_communicator
{
    constexpr int DEFAULT_PORT = 27001;
    bool SendSignal(const std::string& message, uint16_t port = DEFAULT_PORT);

    class InstanceCommunicator
    {
    public:
        InstanceCommunicator(boost::asio::io_context& ioContext,
                             uint16_t port,
                             std::function<void(const std::string&)> handler);

        std::optional<std::string> WaitForSignal();

        void Listen();

        bool SignalReceived() const
        {
            return m_signaled;
        }

    private:
        boost::asio::io_context& m_ioContext;
        uint16_t m_port;
        boost::asio::ip::tcp::acceptor m_acceptor;
        std::atomic<bool> m_signaled;
        std::function<void(const std::string&)> m_signalHandler;
    };
} // namespace instance_communicator
