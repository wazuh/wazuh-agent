#pragma once

#include <boost/asio/awaitable.hpp>

#include <atomic>
#include <chrono>
#include <functional>
#include <string>

namespace logcollector
{
    using Awaitable = boost::asio::awaitable<void>;

    /// @brief Interface for log readers
    class IReader
    {
    public:
        /// @brief Constructor
        IReader(
            std::function<void(const std::string& location, const std::string& log, const std::string& collectorType)>
                pushMessageFunc,
            std::function<Awaitable(std::chrono::milliseconds)> waitFunc)
            : m_pushMessage(pushMessageFunc)
            , m_wait(waitFunc)
        {
        }

        /// @brief Destructor
        virtual ~IReader() = default;

        /// @brief Runs the log reader
        /// @return Awaitable result
        virtual Awaitable Run() = 0;

        /// @brief Stops the log reader
        virtual void Stop() = 0;

    protected:
        /// @brief Push message function
        /// @param message The message to push
        /// @return The result of the push operation
        std::function<void(const std::string& location, const std::string& log, const std::string& collectorType)>
            m_pushMessage;

        /// @brief Wait function
        /// @param waitTime The time to wait
        /// @return Awaitable result
        std::function<Awaitable(std::chrono::milliseconds waitTimeMillis)> m_wait;

        /// @brief Indicates if the log reader should keep running
        std::atomic<bool> m_keepRunning = true;
    };

} // namespace logcollector
