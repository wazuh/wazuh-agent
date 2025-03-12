#pragma once

#include <future>
#include <thread>

namespace Utils
{
    /// @brief Interface for waiting
    class IWait
    {
    public:
        /// @brief Destructor
        virtual ~IWait() = default;

        /// @brief Set value
        virtual void set_value() = 0;

        /// @brief Wait
        virtual void wait() = 0;
    };

    /// @brief Promise waiting
    class PromiseWaiting final : public IWait
    {
        std::promise<void> m_promise;

    public:
        /// @brief Constructor
        explicit PromiseWaiting() {};

        /// @brief Destructor
        virtual ~PromiseWaiting() = default;

        /// @copydoc IWait::set_value
        virtual void set_value() override
        {
            m_promise.set_value();
        }

        /// @copydoc IWait::wait
        virtual void wait() override
        {
            m_promise.get_future().wait();
        }
    };

    /// @brief Busy waiting
    class BusyWaiting final : public IWait
    {
        std::atomic<bool> end;

    public:
        /// @brief Constructor
        explicit BusyWaiting()
            : end {false} {};

        /// @brief Destructor
        virtual ~BusyWaiting() = default;

        /// @copydoc IWait::set_value
        virtual void set_value() override
        {
            end = true;
        }

        /// @copydoc IWait::wait
        virtual void wait() override
        {
            while (!end.load())
            {
                std::this_thread::sleep_for(std::chrono::seconds {1});
            }
        }
    };
} // namespace Utils
