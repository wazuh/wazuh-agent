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
     * @brief Store a JSON message in the storage.
     *
     * @param message The JSON message to store.
     */
    virtual void Store(const json& message) = 0;

    /**
     * @brief Retrieve a JSON message by its ID.
     *
     * @param id The ID of the message to retrieve.
     * @return The retrieved JSON message.
     */
    virtual json Retrieve(int id) = 0;

    /**
     * @brief Retrieve multiple JSON messages.
     *
     * @param n The number of messages to retrieve.
     * @return A vector of retrieved JSON messages.
     */
    virtual json RetrieveMultiple(int n) = 0;

    /**
     * @brief Remove a JSON message by its ID.
     *
     * @param id The ID of the message to remove.
     * @return The number of removed elements.
     */
    virtual int Remove(int id) = 0;

    /**
     * @brief Remove multiple JSON messages.
     *
     * @param n The number of messages to remove.
     * @return The number of removed elements.
     */
    virtual int RemoveMultiple(int n) = 0;

    /**
     * @brief Get the number of elements in the table.
     *
     * @return The number of elements in the table.
     */
    virtual int GetElementCount() = 0;
};

#endif // PERSISTENCE_H
