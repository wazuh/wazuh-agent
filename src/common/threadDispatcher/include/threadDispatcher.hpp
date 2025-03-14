#pragma once

#include "commonDefs.h"
#include "promiseFactory.hpp"
#include "threadSafeQueue.hpp"
#include <atomic>
#include <functional>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

namespace Utils
{
    /// @brief AsyncDispatcher class
    template<typename Type, typename Functor>
    class AsyncDispatcher
    {
    public:
        /// @brief Constructor
        /// @param functor functor
        /// @param numberOfThreads number of threads
        /// @param maxQueueSize max queue size
        AsyncDispatcher(Functor functor,
                        const unsigned int numberOfThreads = std::thread::hardware_concurrency(),
                        const size_t maxQueueSize = UNLIMITED_QUEUE_SIZE)
            : m_functor {functor}
            , m_running {true}
            , m_numberOfThreads {numberOfThreads ? numberOfThreads : 1}
            , m_maxQueueSize {maxQueueSize}
        {
            m_threads.reserve(m_numberOfThreads);

            for (unsigned int i = 0; i < m_numberOfThreads; ++i)
            {
                m_threads.push_back(std::thread {&AsyncDispatcher<Type, Functor>::dispatch, this});
            }
        }

        /// @brief Delete copy assignment operator
        AsyncDispatcher& operator=(const AsyncDispatcher&) = delete;

        /// @brief Delete copy constructor
        AsyncDispatcher(AsyncDispatcher& other) = delete;

        /// @brief Destructor
        ~AsyncDispatcher()
        {
            cancel();
        }

        /// @brief Push an element to the queue
        /// @param value the value
        void push(const Type& value)
        {
            if (m_running)
            {
                if (UNLIMITED_QUEUE_SIZE == m_maxQueueSize || m_queue.size() < m_maxQueueSize)
                {
                    m_queue.push([value, this]() { this->m_functor(value); });
                }
            }
        }

        /// @brief Rundown the dispatcher
        void rundown()
        {
            if (m_running)
            {
                auto promise {PromiseFactory<PROMISE_TYPE>::getPromiseObject()};
                m_queue.push([&promise]() { promise->set_value(); });
                promise->wait();
                cancel();
            }
        }

        /// @brief Cancel the dispatcher
        void cancel()
        {
            m_running = false;
            m_queue.cancel();
            joinThreads();
        }

        /// @brief Checks if the dispatcher is canceled
        /// @return true if the dispatcher is canceled
        bool cancelled() const
        {
            return !m_running;
        }

        /// @brief Get the number of threads
        /// @return number of threads
        unsigned int numberOfThreads() const
        {
            return m_numberOfThreads;
        }

        /// @brief Get the size of the queue
        /// @return the size of the queue
        size_t size() const
        {
            return m_queue.size();
        }

    private:
        /// @brief Dispatch handler
        void dispatch()
        {
            try
            {
                while (m_running)
                {
                    std::function<void()> fnc;

                    if (m_queue.pop(fnc))
                    {
                        fnc();
                    }
                }
            }
            catch (const std::exception& ex)
            {
                std::cerr << "Dispatch handler error, " << ex.what() << std::endl;
            }
        }

        /// @brief Join the threads
        void joinThreads()
        {
            for (auto& thread : m_threads)
            {
                if (thread.joinable())
                {
                    thread.join();
                }
            }
        }

        Functor m_functor;
        SafeQueue<std::function<void()>> m_queue;
        std::vector<std::thread> m_threads;
        std::atomic_bool m_running;
        const unsigned int m_numberOfThreads;
        const size_t m_maxQueueSize;
    };

    /// @brief SyncDispatcher class
    template<typename Input, typename Functor>
    class SyncDispatcher
    {
    public:
        /// @brief Constructor
        /// @param functor functor
        SyncDispatcher(Functor functor,
                       const unsigned int /*numberOfThreads = std::thread::hardware_concurrency()*/,
                       const size_t /*maxQueueSize = UNLIMITED_QUEUE_SIZE*/)
            : m_functor {functor}
            , m_running {true}
        {
        }

        /// @brief Constructor
        /// @param functor functor
        SyncDispatcher(Functor functor)
            : m_functor {functor}
            , m_running {true}
        {
        }

        /// @brief Push an element to the queue
        /// @param data the data
        void push(const Input& data)
        {
            if (m_running)
            {
                m_functor(data);
            }
        }

        /// @brief Get the size of the queue
        /// @return the size of the queue
        size_t size() const
        {
            return 0;
        }

        /// @brief Rundown the dispatcher
        void rundown()
        {
            cancel();
        }

        /// @brief Cancel the dispatcher
        void cancel()
        {
            m_running = false;
        }

        /// @brief Checks if the dispatcher is canceled
        /// @return true if the dispatcher is canceled
        bool cancelled() const
        {
            return !m_running;
        }

        /// @brief Get the number of threads
        /// @return number of threads
        unsigned int numberOfThreads() const
        {
            return 0;
        }

        /// @brief Destructor
        ~SyncDispatcher() = default;

    private:
        Functor m_functor;
        bool m_running;
    };
} // namespace Utils
