#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

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
     * @brief
     *
     * @param message
     * @param queueName
     * @param moduleName
     * @return int
     */
    virtual int Store(const json& message, const std::string& queueName, const std::string& moduleName = "") = 0;

    /**
     * @brief
     *
     * @param id
     * @param queueName
     * @param moduleName
     * @return json
     */
    virtual json Retrieve(int id, const std::string& queueName, const std::string& moduleName = "") = 0;

    /**
     * @brief
     *
     * @param n
     * @param queueName
     * @param moduleName
     * @return json
     */
    virtual json RetrieveMultiple(int n, const std::string& queueName, const std::string& moduleName = "") = 0;

    /**
     * @brief
     *
     * @param id
     * @param queueName
     * @param moduleName
     * @return int
     */
    virtual int Remove(int id, const std::string& queueName, const std::string& moduleName = "") = 0;

    /**
     * @brief
     *
     * @param n
     * @param queueName
     * @param moduleName
     * @return int
     */
    virtual int RemoveMultiple(int n, const std::string& queueName, const std::string& moduleName = "") = 0;

    /**
     * @brief Get the Element Count object
     *
     * @param queueName
     * @param moduleName
     * @return int
     */
    virtual int GetElementCount(const std::string& queueName, const std::string& moduleName = "") = 0;
};

#endif // PERSISTENCE_H
