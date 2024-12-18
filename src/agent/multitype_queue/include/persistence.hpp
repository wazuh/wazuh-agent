#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

/**
 * @brief Interface for persistence storage.
 *
 * This interface defines methods for storing, retrieving, and removing JSON messages.
 */
class Persistence
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~Persistence() = default;

    /**
     * @brief Store a JSON message in the specified queue.
     *
     * @param message The JSON message to be stored.
     * @param queueName The name of the queue.
     * @param moduleName The name of the module.
     * @param moduleType The type of the module.
     * @param metadata The metadata message to store.
     * @return int The number of messages stored.
     */
    virtual int Store(const nlohmann::json& message,
                      const std::string& tableName,
                      const std::string& moduleName = "",
                      const std::string& moduleType = "",
                      const std::string& metadata = "") = 0;

    /**
     * @brief Retrieve multiple JSON messages from the specified queue.
     *
     * @param n number of messages to be retrieved.
     * @param queueName The name of the queue.
     * @param moduleName The name of the module.
     * @param moduleType The type of the module.
     * @return nlohmann::json The retrieved JSON messages.
     */
    virtual nlohmann::json RetrieveMultiple(int n,
                                            const std::string& queueName,
                                            const std::string& moduleName = "",
                                            const std::string& moduleType = "") = 0;

    /**
     * @brief Remove a JSON message from the specified queue.
     *
     * @param id number of messages to be removed.
     * @param queueName The name of the queue.
     * @param moduleName The name of the module.
     * @return int The number of messages removed.
     */
    virtual int Remove(int id, const std::string& queueName, const std::string& moduleName = "") = 0;

    /**
     * @brief Remove multiple JSON messages from the specified queue.
     *
     * @param n number of messages to be removed.
     * @param queueName The name of the queue.
     * @param moduleName The name of the module.
     * @return int The number of messages removed.
     */
    virtual int RemoveMultiple(int n, const std::string& queueName, const std::string& moduleName = "") = 0;

    /**
     * @brief Get the quantity of elements stored in the specified queue.
     *
     * @param queueName The name of the queue.
     * @param moduleName The name of the module.
     * @return int The quantity of elements stored in the specified queue.
     */
    virtual int GetElementCount(const std::string& queueName, const std::string& moduleName = "") = 0;

    /**
     * @brief Get the bytes occupied by elements stored in the specified queue.
     *
     * @param queueName The name of the queue.
     * @return size_t The bytes occupied by elements stored in the specified queue.
     */
    virtual size_t GetElementsStoredSize(const std::string& queueName) = 0;

    /**
     * @brief Retrieve multiple JSON messages based on size from the specified queue.
     *
     * @param n size occupied by the messages to be retrieved.
     * @param queueName The name of the queue.
     * @param moduleName The name of the module.
     * @param moduleType The type of the module.
     * @return nlohmann::json The retrieved JSON messages.
     */
    virtual nlohmann::json RetrieveBySize(size_t n,
                                          const std::string& queueName,
                                          const std::string& moduleName = "",
                                          const std::string& moduleType = "") = 0;
};
