#include <config.h>
#include <multitype_queue.hpp>
#include <storage.hpp>

#include <boost/asio.hpp>

#include <logger.hpp>

#include <stop_token>
#include <utility>

namespace
{
    constexpr auto MIN_BATCH_INTERVAL = 1000;
    constexpr auto MAX_BATCH_INTERVAL = 60 * 60 * 1000;
    constexpr auto MIN_QUEUE_SIZE = 1000;
    constexpr auto MAX_QUEUE_SIZE = 60 * 60 * 1000;
} // namespace

MultiTypeQueue::MultiTypeQueue(std::shared_ptr<configuration::ConfigurationParser> configurationParser)
    : m_timeout(config::agent::QUEUE_STATUS_REFRESH_TIMER)
{
    if (!configurationParser)
    {
        throw std::runtime_error(std::string("Invalid Configuration Parser passed."));
    }

    m_batchInterval = configurationParser->GetTimeConfigInRangeOrDefault(
        config::agent::DEFAULT_BATCH_INTERVAL, MIN_BATCH_INTERVAL, MAX_BATCH_INTERVAL, "events", "batch_interval");

    const auto configMaxItems =
        configurationParser->GetConfigOrDefault(config::agent::QUEUE_DEFAULT_SIZE, "agent", "queue_size");

    m_maxItems = configurationParser->ParseSizeUnit(configMaxItems);

    if (m_maxItems < MIN_QUEUE_SIZE || m_maxItems > MAX_QUEUE_SIZE)
    {
        LogWarn("queue_size must be between 1'000 and 100'000'000. Using default value {}.",
                config::agent::QUEUE_DEFAULT_SIZE);
        m_maxItems = configurationParser->ParseSizeUnit(config::agent::QUEUE_DEFAULT_SIZE);
    }

    const auto dbFolderPath = configurationParser->GetConfigOrDefault(config::DEFAULT_DATA_PATH, "agent", "path.data");

    try
    {
        m_persistenceDest = std::make_unique<Storage>(dbFolderPath, m_vMessageTypeStrings);
    }
    catch (const std::exception& e)
    {
        LogError("Error creating persistence: {}.", e.what());
    }
}

MultiTypeQueue::~MultiTypeQueue() = default;

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
            m_cv.wait_for(lock,
                          m_timeout,
                          [&, this] {
                              return static_cast<size_t>(m_persistenceDest->GetElementCount(sMessageType)) < m_maxItems;
                          });
        }

        const auto storedMessages = static_cast<size_t>(m_persistenceDest->GetElementCount(sMessageType));
        const auto spaceAvailable = (m_maxItems > storedMessages) ? m_maxItems - storedMessages : 0;
        if (spaceAvailable)
        {
            auto messageData = message.data;
            if (messageData.is_array())
            {
                if (messageData.size() <= spaceAvailable)
                {
                    for (const auto& singleMessageData : messageData)
                    {
                        result += m_persistenceDest->Store(
                            singleMessageData, sMessageType, message.moduleName, message.moduleType, message.metaData);
                        m_cv.notify_all();
                    }
                }
            }
            else
            {
                result = m_persistenceDest->Store(message.data,
                                                  m_mapMessageTypeName.at(message.type),
                                                  message.moduleName,
                                                  message.moduleType,
                                                  message.metaData);
                m_cv.notify_all();
            }
        }
    }
    else
    {
        LogError("Error didn't find the queue.");
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

        while (static_cast<size_t>(m_persistenceDest->GetElementCount(sMessageType)) >= m_maxItems)
        {
            timer.expires_after(std::chrono::milliseconds(m_timeout));
            co_await timer.async_wait(boost::asio::use_awaitable);
        }

        const auto storedItems = static_cast<size_t>(m_persistenceDest->GetElementCount(sMessageType));
        const auto availableItems = (m_maxItems > storedItems) ? m_maxItems - storedItems : 0;
        if (availableItems)
        {
            auto messageData = message.data;
            if (messageData.is_array())
            {
                if (messageData.size() <= availableItems)
                {
                    for (const auto& singleMessageData : messageData)
                    {
                        result += m_persistenceDest->Store(
                            singleMessageData, sMessageType, message.moduleName, message.moduleType, message.metaData);
                        m_cv.notify_all();
                    }
                }
            }
            else
            {
                result = m_persistenceDest->Store(message.data,
                                                  m_mapMessageTypeName.at(message.type),
                                                  message.moduleName,
                                                  message.moduleType,
                                                  message.metaData);
                m_cv.notify_all();
            }
        }
    }
    else
    {
        LogError("Error didn't find the queue.");
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

Message MultiTypeQueue::getNext(MessageType type, const std::string moduleName, const std::string moduleType)
{
    Message result(type, "{}"_json, moduleName, moduleType, "");
    if (m_mapMessageTypeName.contains(type))
    {
        auto resultData = m_persistenceDest->RetrieveMultiple(1, m_mapMessageTypeName.at(type), moduleName, moduleType);
        if (!resultData.empty())
        {
            result.data = resultData[0]["data"];
            result.metaData = resultData[0]["metadata"];
            result.moduleName = resultData[0]["moduleName"];
            result.moduleType = resultData[0]["moduleType"];
        }
    }
    else
    {
        LogError("Error didn't find the queue.");
    }
    return result;
}

boost::asio::awaitable<std::vector<Message>> MultiTypeQueue::getNextBytesAwaitable(MessageType type,
                                                                                   const size_t messageQuantity,
                                                                                   const std::string moduleName,
                                                                                   const std::string moduleType)
{
    boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);

    std::vector<Message> result;
    if (m_mapMessageTypeName.contains(type))
    {
        //  waits for specified size stored
        boost::asio::steady_timer batchTimeoutTimer(co_await boost::asio::this_coro::executor);
        batchTimeoutTimer.expires_after(std::chrono::milliseconds(m_batchInterval));

        while ((sizePerType(type) < messageQuantity) && (batchTimeoutTimer.expiry() > std::chrono::steady_clock::now()))
        {
            timer.expires_after(std::chrono::milliseconds(m_timeout));
            co_await timer.async_wait(boost::asio::use_awaitable);
        }

        if (sizePerType(type) >= messageQuantity)
        {
            LogDebug("Required size achieved: {}B", messageQuantity);
        }
        else
        {
            LogDebug("Timeout reached after {}ms", m_batchInterval);
        }

        result = getNextBytes(type, messageQuantity, moduleName, moduleType);
    }
    else
    {
        LogError("Error didn't find the queue.");
    }
    co_return result;
}

std::vector<Message> MultiTypeQueue::getNextBytes(MessageType type,
                                                  const size_t messageQuantity,
                                                  const std::string moduleName,
                                                  const std::string moduleType)
{
    std::vector<Message> result;
    if (m_mapMessageTypeName.contains(type))
    {
        auto arrayData =
            m_persistenceDest->RetrieveBySize(messageQuantity, m_mapMessageTypeName.at(type), moduleName, moduleType);

        for (auto singleJson : arrayData)
        {
            result.emplace_back(
                type, singleJson["data"], singleJson["moduleName"], singleJson["moduleType"], singleJson["metadata"]);
        }
    }
    else
    {
        LogError("Error didn't find the queue.");
    }
    return result;
}

bool MultiTypeQueue::pop(MessageType type, const std::string moduleName, const std::string moduleType)
{
    bool result = false;
    if (m_mapMessageTypeName.contains(type))
    {
        result = m_persistenceDest->RemoveMultiple(1, m_mapMessageTypeName.at(type), moduleName, moduleType);
    }
    else
    {
        LogError("Error didn't find the queue.");
    }
    return result;
}

int MultiTypeQueue::popN(MessageType type,
                         int messageQuantity,
                         const std::string moduleName,
                         const std::string moduleType)
{
    int result = 0;
    if (m_mapMessageTypeName.contains(type))
    {
        result =
            m_persistenceDest->RemoveMultiple(messageQuantity, m_mapMessageTypeName.at(type), moduleName, moduleType);
    }
    else
    {
        LogError("Error didn't find the queue.");
    }
    return result;
}

bool MultiTypeQueue::isEmpty(MessageType type, const std::string moduleName, const std::string moduleType)
{
    if (m_mapMessageTypeName.contains(type))
    {
        return m_persistenceDest->GetElementCount(m_mapMessageTypeName.at(type), moduleName, moduleType) == 0;
    }
    else
    {
        LogError("Error didn't find the queue.");
    }
    return false;
}

bool MultiTypeQueue::isFull(MessageType type, const std::string moduleName, const std::string moduleType)
{
    if (m_mapMessageTypeName.contains(type))
    {
        return static_cast<size_t>(m_persistenceDest->GetElementCount(
                   m_mapMessageTypeName.at(type), moduleName, moduleType)) == m_maxItems;
    }
    else
    {
        LogError("Error didn't find the queue.");
    }
    return false;
}

int MultiTypeQueue::storedItems(MessageType type, const std::string moduleName, const std::string moduleType)
{
    if (m_mapMessageTypeName.contains(type))
    {
        return m_persistenceDest->GetElementCount(m_mapMessageTypeName.at(type), moduleName, moduleType);
    }
    else
    {
        LogError("Error didn't find the queue.");
    }
    return false;
}

size_t MultiTypeQueue::sizePerType(MessageType type)
{
    if (m_mapMessageTypeName.contains(type))
    {
        return m_persistenceDest->GetElementsStoredSize(m_mapMessageTypeName.at(type));
    }
    else
    {
        LogError("Error didn't find the queue.");
    }
    return false;
}
