#pragma once

#include <boost/asio/awaitable.hpp>

#include <functional>
#include <string>

class ISCAPolicy
{
public:
    /// @brief Destructor
    virtual ~ISCAPolicy() = default;

    /// @brief Runs the policy check
    /// @param scanInterval Scan interval in milliseconds
    /// @param scanOnStart Scan on start
    /// @param reportCheckResult Function to report check result
    /// @param wait Function to wait for the next scan
    /// @return Awaitable void
    virtual boost::asio::awaitable<void>
    Run(std::time_t scanInterval,
        bool scanOnStart,
        std::function<void(const std::string&, const std::string&, const std::string&)> reportCheckResult,
        std::function<boost::asio::awaitable<void>(std::chrono::milliseconds)> wait) = 0;

    /// @brief Stops the policy check
    virtual void Stop() = 0;
};
