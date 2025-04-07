#include <instance_communicator.hpp>

#include <logger.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include <iostream>

namespace instance_communicator
{
    InstanceCommunicator::InstanceCommunicator(std::function<void(const std::string&)> handler)
        : m_signalHandler(std::move(handler))
    {
    }

    void InstanceCommunicator::Stop()
    {
        m_keepRunning.store(false);
    }
} // namespace instance_communicator
