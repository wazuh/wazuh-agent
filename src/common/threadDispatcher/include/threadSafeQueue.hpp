#pragma once

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace Utils
{
    /// @brief TSafeQueue class
    template<typename T, typename U, typename Tq = std::queue<T>>
    class TSafeQueue
    {
    public:
        /// @brief Constructor
        TSafeQueue()
            : m_canceled {false}
        {
        }

        /// @brief Delete copy assignment operator
        TSafeQueue& operator=(const TSafeQueue&) = delete;

        /// @brief Copy constructor
        /// @param other object
        TSafeQueue(TSafeQueue& other)
            : TSafeQueue {}
        {
            std::lock_guard<std::mutex> lock {other.m_mutex};
            m_queue = other.m_queue;
        }

        /// @brief Move constructor
        /// @param queue the queue
        explicit TSafeQueue(Tq&& queue)
            : m_queue {std::move(queue)}
            , m_canceled {false}
        {
        }

        /// @brief Destructor
        ~TSafeQueue()
        {
            cancel();
        }

        /// @brief Push an element to the queue
        /// @param value the value
        void push(const T& value)
        {
            std::lock_guard<std::mutex> lock {m_mutex};

            if (!m_canceled)
            {
                m_queue.push(value);
                m_cv.notify_one();
            }
        }

        /// @brief Pops an element from the queue
        /// @param value the value
        /// @param wait wait for an element
        /// @return true if an element was popped
        bool pop(U& value, const bool wait = true)
        {
            std::unique_lock<std::mutex> lock {m_mutex};

            if (wait)
            {
                m_cv.wait(lock, [this]() { return !m_queue.empty() || m_canceled; });
            }

            const bool ret {!m_canceled && !m_queue.empty()};

            if (ret)
            {
                value = std::move(m_queue.front());
                m_queue.pop();
            }

            return ret;
        }

        /// @brief Pops an element from the queue and returns it
        /// @param wait wait for an element
        /// @return the element
        std::shared_ptr<U> pop(const bool wait = true)
        {
            std::unique_lock<std::mutex> lock {m_mutex};

            if (wait)
            {
                m_cv.wait(lock, [this]() { return !m_queue.empty() || m_canceled; });
            }

            const bool ret {!m_canceled && !m_queue.empty()};

            if (ret)
            {
                const auto spData {std::make_shared<U>(m_queue.front())};
                m_queue.pop();
                return spData;
            }

            return nullptr;
        }

        /// @brief Gets a bulk of elements from the queue
        /// @param elementsQuantity the number of elements to get
        /// @param timeout the timeout for the wait
        /// @return the bulk
        std::queue<U> getBulk(const uint64_t elementsQuantity,
                              const std::chrono::seconds& timeout = std::chrono::seconds(5))
        {
            std::unique_lock<std::mutex> lock {m_mutex};
            std::queue<U> bulkQueue;

            // If we have less elements than requested, wait for more elements to be pushed.
            // coverity[missing_lock]
            if (m_queue.size() < elementsQuantity)
            {
                m_cv.wait_for(lock,
                              timeout,
                              [this, elementsQuantity]()
                              {
                                  // coverity[missing_lock]
                                  return m_canceled.load() || m_queue.size() >= elementsQuantity;
                              });
            }

            // If the queue is not canceled, get the elements.
            if (!m_canceled)
            {
                for (auto i = 0; i < elementsQuantity && i < m_queue.size(); ++i)
                {
                    bulkQueue.push(std::move(m_queue.at(i)));
                }
            }

            return bulkQueue;
        }

        /// @brief Pops a bulk of elements from the queue
        /// @param elementsQuantity the number of elements to pop
        void popBulk(const uint64_t elementsQuantity)
        {
            std::lock_guard<std::mutex> lock {m_mutex};
            for (auto i = 0; i < elementsQuantity && !m_queue.empty(); ++i)
            {
                m_queue.pop();
            }
        }

        /// @brief Checks if the queue is empty
        /// @return true if the queue is empty
        bool empty() const
        {
            std::lock_guard<std::mutex> lock {m_mutex};
            return m_queue.empty();
        }

        /// @brief Gets the size of the queue
        /// @return the size
        size_t size() const
        {
            std::lock_guard<std::mutex> lock {m_mutex};
            return m_queue.size();
        }

        /// @brief Cancel the queue
        void cancel()
        {
            std::lock_guard<std::mutex> lock {m_mutex};
            m_canceled = true;
            m_cv.notify_all();
        }

        /// @brief Checks if the queue is canceled
        /// @return true if the queue is canceled
        bool cancelled() const
        {
            std::lock_guard<std::mutex> lock {m_mutex};
            return m_canceled;
        }

    private:
        mutable std::mutex m_mutex;
        std::condition_variable m_cv;
        std::atomic<bool> m_canceled {};
        Tq m_queue;
    };

    template<typename T, typename Tq = std::queue<T>>
    using SafeQueue = TSafeQueue<T, T, Tq>;
} // namespace Utils
