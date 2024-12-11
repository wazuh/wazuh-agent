#include <multitype_queue.hpp>

#include <config.h>
#include <logger.hpp>
#include <persistence_factory.hpp>

#include <stop_token>
#include <utility>

constexpr int DEFAULT_TIMER_IN_MS = 100;

MultiTypeQueue::MultiTypeQueue(const std::string& dbFolderPath, size_t size, int timeout)
    : m_maxItems(size)
    , m_timeout(timeout)
{
    const auto dbFilePath = dbFolderPath + "/" + QUEUE_DB_NAME;

    // const ConfigGetter& getConfigValue

    // m_batchInterval = getConfigValue.template operator()<std::time_t>("events", "batch_interval")
    //                         .value_or(config::agent::DEFAULT_BATCH_INTERVAL);

    // if (m_batchInterval < 1'000 || m_batchInterval > (1'000 * 60 * 60))
    // {
    //     LogWarn("batch_interval must be between 1s and 1h. Using default value.");
    //     m_batchInterval = config::agent::DEFAULT_BATCH_INTERVAL;
    // }

    //     /// @brief Time between batch requests
    // std::time_t m_batchInterval = config::agent::DEFAULT_BATCH_INTERVAL;

    try
    {
        m_persistenceDest = PersistenceFactory::createPersistence(PersistenceFactory::PersistenceType::SQLITE3,
                                                                  {dbFilePath, m_vMessageTypeStrings});
    }
    catch (const std::exception& e)
    {
        LogError("Error creating persistence: {}.", e.what());
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
            timer.expires_after(std::chrono::milliseconds(DEFAULT_TIMER_IN_MS));
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

boost::asio::awaitable<std::vector<Message>>
MultiTypeQueue::getNextNAwaitable(MessageType type,
                                  std::variant<const int, const size_t> messageQuantity,
                                  const std::string moduleName,
                                  const std::string moduleType)
{
    boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);

    std::vector<Message> result;
    if (m_mapMessageTypeName.contains(type))
    {
        if (std::holds_alternative<const int>(messageQuantity))
        {
            // waits for items to be available
            while (isEmpty(type))
            {
                timer.expires_after(std::chrono::milliseconds(DEFAULT_TIMER_IN_MS));
                co_await timer.async_wait(boost::asio::use_awaitable);
            }
        }
        else if (std::holds_alternative<const size_t>(messageQuantity))
        {
            // waits for specified size stored
            size_t sizeRequested = std::get<const size_t>(messageQuantity);

            boost::asio::steady_timer batchTimeoutTimer(co_await boost::asio::this_coro::executor);
            // TODO: make it variable through configuration
            constexpr int DELAY_MAX = 10000;
            batchTimeoutTimer.expires_after(std::chrono::milliseconds(DELAY_MAX));

            while ((sizePerType(type) < sizeRequested) &&
                   (batchTimeoutTimer.expiry() > std::chrono::steady_clock::now()))
            {
                timer.expires_after(std::chrono::milliseconds(DEFAULT_TIMER_IN_MS));
                co_await timer.async_wait(boost::asio::use_awaitable);
            }

            if (sizePerType(type) >= sizeRequested)
            {
                LogDebug("Required size achieved: {}B", sizeRequested);
            }
            else
            {
                LogDebug("Timeout reached after {}ms", DELAY_MAX);
            }
        }
        else
        {
            LogError("Unexpected variant type on messageQuantity");
        }

        result = getNextN(type, messageQuantity, moduleName, moduleType);
    }
    else
    {
        LogError("Error didn't find the queue.");
    }
    co_return result;
}

std::vector<Message> MultiTypeQueue::getNextN(MessageType type,
                                              std::variant<const int, const size_t> messageQuantity,
                                              const std::string moduleName,
                                              const std::string moduleType)
{
    std::vector<Message> result;
    if (m_mapMessageTypeName.contains(type))
    {
        nlohmann::json arrayData;
        if (std::holds_alternative<const int>(messageQuantity))
        {
            arrayData = m_persistenceDest->RetrieveMultiple(
                std::get<const int>(messageQuantity), m_mapMessageTypeName.at(type), moduleName, moduleType);
        }
        else if (std::holds_alternative<const size_t>(messageQuantity))
        {
            LogInfo("Requesting {}B ", std::get<const size_t>(messageQuantity));
            arrayData = m_persistenceDest->RetrieveBySize(
                std::get<const size_t>(messageQuantity), m_mapMessageTypeName.at(type), moduleName, moduleType);
        }
        else
        {
            LogError("Unexpected variant type on messageQuantity");
        }

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

bool MultiTypeQueue::pop(MessageType type, const std::string moduleName)
{
    bool result = false;
    if (m_mapMessageTypeName.contains(type))
    {
        result = m_persistenceDest->RemoveMultiple(1, m_mapMessageTypeName.at(type), moduleName);
    }
    else
    {
        LogError("Error didn't find the queue.");
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
        LogError("Error didn't find the queue.");
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
        LogError("Error didn't find the queue.");
    }
    return false;
}

bool MultiTypeQueue::isFull(MessageType type, const std::string moduleName)
{
    if (m_mapMessageTypeName.contains(type))
    {
        return static_cast<size_t>(m_persistenceDest->GetElementCount(m_mapMessageTypeName.at(type), moduleName)) ==
               m_maxItems;
    }
    else
    {
        LogError("Error didn't find the queue.");
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
