#pragma once

#include "pipelinePattern.hpp"
#include "threadDispatcher.h"
#include <functional>

namespace Utils
{
    /// @brief Read Node class template
    /// @tparam Input Message type
    /// @tparam Functor Entity that processes the messages
    template<typename Input,
             typename Functor = std::function<void(const Input&)>,
             template<class, class> class Dispatcher = SyncDispatcher>
    class ReadNode : public Dispatcher<Input, Functor>
    {
    public:
        /// @brief Constructor
        /// @param functor Entity that processes the messages
        ReadNode(Functor functor)
            : DispatcherType {functor}
        {
        }

        /// @brief Constructor
        /// @param functor Entity that processes the messages
        /// @param numberOfThreads Number of threads
        ReadNode(Functor functor, const unsigned int numberOfThreads)
            : DispatcherType {functor, numberOfThreads, UNLIMITED_QUEUE_SIZE}
        {
        }

        /// @brief Destructor
        ~ReadNode() = default;

        /// @brief Receive a message
        /// @param data Message
        void receive(const Input& data)
        {
            DispatcherType::push(data);
        }

    private:
        using DispatcherType = Dispatcher<Input, Functor>;
        using ReadNodeType = ReadNode<Input, Functor, Dispatcher>;
    };

    /// @brief Read Write Node class template
    /// @tparam Input Message type
    /// @tparam Output Message type
    template<typename Input,
             typename Output,
             typename Reader,
             typename Functor = std::function<Output(const Input&)>,
             template<class, class> class Dispatcher = SyncDispatcher>
    class ReadWriteNode
        : public Utils::IPipelineWriter<Output, Reader>
        , public Dispatcher<Input, std::function<void(const Input&)>>
    {
    public:
        /// @brief Constructor
        /// @param functor Entity that processes the messages
        ReadWriteNode(Functor functor)
            : DispatcherType {std::bind(&RWNodeType::doTheWork, this, std::placeholders::_1)}
            , m_functor {functor}
        {
        }

        /// @brief Constructor
        /// @param functor Entity that processes the messages
        /// @param numberOfThreads Number of threads
        ReadWriteNode(Functor functor, const unsigned int numberOfThreads)
            : DispatcherType {std::bind(&RWNodeType::doTheWork, this, std::placeholders::_1),
                              numberOfThreads,
                              UNLIMITED_QUEUE_SIZE}
            , m_functor {functor}
        {
        }

        /// @brief Destructor
        ~ReadWriteNode() = default;

        /// @brief Receive a message
        /// @param data Message
        void receive(const Input& data)
        {
            DispatcherType::push(data);
        }

    private:
        using DispatcherType = Dispatcher<Input, std::function<void(const Input&)>>;
        using RWNodeType = ReadWriteNode<Input, Output, Reader, Functor, Dispatcher>;
        using WriterType = Utils::IPipelineWriter<Output, Reader>;

        /// @brief Does the work of the node
        /// @param data Message
        void doTheWork(const Input& data)
        {
            WriterType::send(m_functor(data));
        }

        Functor m_functor;
    };
} // namespace Utils
