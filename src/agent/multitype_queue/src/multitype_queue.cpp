#include <multitype_queue.hpp>

#include <persistence_factory.hpp>

#include <chrono>
#include <iostream>
#include <stop_token>
#include <utility>

MultiTypeQueue::MultiTypeQueue(int size, int timeout)
    : m_maxItems(size)
    , m_timeout(timeout)
{
    try
    {
        m_persistenceDest = PersistenceFactory::createPersistence(
            PersistenceFactory::PersistenceType::SQLITE3,
            {static_cast<std::string>(QUEUE_DEFAULT_DB_PATH), m_vMessageTypeStrings});
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error creating persistence: " << e.what() << '\n';
    }
}

int MultiTypeQueue::push(Message message, bool shouldWait)
{
    int result = 0;

    if (m_mapMessageTypeName.contains(message.type))
    {
        auto sMessageType = m_mapMessageTypeName.at(message.type);

        // Wait until the queue is not full
        if (shouldWait)
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.wait_for(
                lock, m_timeout, [&, this] { return m_persistenceDest->GetElementCount(sMessageType) < m_maxItems; });
        }

        auto storedMessages = m_persistenceDest->GetElementCount(sMessageType);
        size_t spaceAvailable = (m_maxItems > storedMessages) ? m_maxItems - storedMessages : 0;
        if (spaceAvailable)
        {
            auto messageData = message.data;
            if (messageData.is_array())
            {
                if (messageData.size() <= spaceAvailable)
                {
                    for (const auto& singleMessageData : messageData)
                    {
                        result += m_persistenceDest->Store(singleMessageData, sMessageType, message.moduleName);
                        m_cv.notify_all();
                    }
                }
            }
            else
            {
                result =
                    m_persistenceDest->Store(message.data, m_mapMessageTypeName.at(message.type), message.moduleName);
                m_cv.notify_all();
            }
        }
    }
    else
    {
        std::cout << "error didn't find the queue" << std::endl;
    }
    return result;
}

boost::asio::awaitable<int> MultiTypeQueue::pushAwaitable(Message message)
{
    int result = 0;
    boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);

    if (m_mapMessageTypeName.contains(message.type))
    {
        auto sMessageType = m_mapMessageTypeName.at(message.type);

        while (m_persistenceDest->GetElementCount(sMessageType) >= m_maxItems)
        {
            timer.expires_after(std::chrono::milliseconds(100));
            co_await timer.async_wait(boost::asio::use_awaitable);
        }

        auto storedMessages = m_persistenceDest->GetElementCount(sMessageType);
        size_t spaceAvailable = (m_maxItems > storedMessages) ? m_maxItems - storedMessages : 0;
        if (spaceAvailable)
        {
            auto messageData = message.data;
            if (messageData.is_array())
            {
                if (messageData.size() <= spaceAvailable)
                {
                    for (const auto& singleMessageData : messageData)
                    {
                        result += m_persistenceDest->Store(singleMessageData, sMessageType, message.moduleName);
                        m_cv.notify_all();
                    }
                }
            }
            else
            {
                result =
                    m_persistenceDest->Store(message.data, m_mapMessageTypeName.at(message.type), message.moduleName);
                m_cv.notify_all();
            }
        }
    }
    else
    {
        std::cout << "error didn't find the queue" << std::endl;
    }
    co_return result;
}

int MultiTypeQueue::push(std::vector<Message> messages)
{
    int result = 0;
    for (const auto& singleMessage : messages)
    {
        result += push(singleMessage);
    }
    return result;
}

Message MultiTypeQueue::getNext(MessageType type, const std::string moduleName)
{
    Message result(type, "{}"_json, moduleName);
    if (m_mapMessageTypeName.contains(type))
    {
        auto resultData = m_persistenceDest->RetrieveMultiple(1, m_mapMessageTypeName.at(type), moduleName);
        if (!resultData.empty())
        {
            result.data = resultData;
            if (moduleName.empty())
            {
                result.moduleName = result.data.at(0).at("module");
            }
        }
    }
    else
    {
        // TODO: error handling and logging
        std::cout << "error didn't find the queue" << std::endl;
    }
    return result;
}

boost::asio::awaitable<Message>
MultiTypeQueue::getNextNAwaitable(MessageType type, int messageQuantity, const std::string moduleName)
{
    boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);

    Message result(type, "{}"_json, moduleName);
    if (m_mapMessageTypeName.contains(type))
    {
        while (isEmpty(type))
        {
            timer.expires_after(std::chrono::milliseconds(100));
            co_await timer.async_wait(boost::asio::use_awaitable);
        }

        auto resultData =
            m_persistenceDest->RetrieveMultiple(messageQuantity, m_mapMessageTypeName.at(type), moduleName);
        if (!resultData.empty())
        {
            result.data = resultData;
            if (moduleName.empty())
            {
                result.moduleName = result.data.at(0).at("module");
            }
        }
    }
    else
    {
        // TODO: error handling and logging
        std::cout << "error didn't find the queue" << std::endl;
    }
    co_return result;
}

std::vector<Message> MultiTypeQueue::getNextN(MessageType type, int messageQuantity, const std::string moduleName)
{
    std::vector<Message> result;
    if (m_mapMessageTypeName.contains(type))
    {
        auto arrayData =
            m_persistenceDest->RetrieveMultiple(messageQuantity, m_mapMessageTypeName.at(type), moduleName);
        for (auto singleJson : arrayData)
        {
            auto finalModuleName = moduleName;
            if (moduleName.empty())
            {
                finalModuleName = singleJson["module"];
            }
            result.push_back(Message(type, singleJson, finalModuleName));
        }
    }
    else
    {
        // TODO: error handling and logging
        std::cout << "error didn't find the queue" << std::endl;
    }
    return result;
}

bool MultiTypeQueue::pop(MessageType type, const std::string moduleName)
{
    bool result = false;
    if (m_mapMessageTypeName.contains(type))
    {
        result = m_persistenceDest->RemoveMultiple(1, m_mapMessageTypeName.at(type), moduleName);
    }
    else
    {
        // TODO: error handling and logging
        std::cout << "error didn't find the queue" << std::endl;
    }
    return result;
}

int MultiTypeQueue::popN(MessageType type, int messageQuantity, const std::string moduleName)
{
    int result = 0;
    if (m_mapMessageTypeName.contains(type))
    {
        result = m_persistenceDest->RemoveMultiple(messageQuantity, m_mapMessageTypeName.at(type), moduleName);
    }
    else
    {
        // TODO: error handling and logging
        std::cout << "error didn't find the queue" << std::endl;
    }
    return result;
}

bool MultiTypeQueue::isEmpty(MessageType type, const std::string moduleName)
{
    if (m_mapMessageTypeName.contains(type))
    {
        return m_persistenceDest->GetElementCount(m_mapMessageTypeName.at(type), moduleName) == 0;
    }
    else
    {
        // TODO: error handling and logging
        std::cout << "error didn't find the queue" << std::endl;
    }
    return false;
}

bool MultiTypeQueue::isFull(MessageType type, const std::string moduleName)
{
    if (m_mapMessageTypeName.contains(type))
    {
        return m_persistenceDest->GetElementCount(m_mapMessageTypeName.at(type), moduleName) == m_maxItems;
    }
    else
    {
        // TODO: error handling and logging
        std::cout << "error didn't find the queue" << std::endl;
    }
    return false;
}

int MultiTypeQueue::storedItems(MessageType type, const std::string moduleName)
{
    if (m_mapMessageTypeName.contains(type))
    {
        return m_persistenceDest->GetElementCount(m_mapMessageTypeName.at(type), moduleName);
    }
    else
    {
        // TODO: error handling and logging
        std::cout << "error didn't find the queue" << std::endl;
    }
    return false;
}
