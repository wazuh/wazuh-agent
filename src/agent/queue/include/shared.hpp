#ifndef SHARED_H
#define SHARED_H

#include <map>

#include <nlohmann/json.hpp>

// TODO: should be moved to Config
constexpr char DEFAULT_FILE_PATH[] = "/home/vagrant/FILE_";
constexpr char DEFAULT_DB_PATH[] = "queue.db";

/**
 * @brief Types of messages enum
 *
 */
enum MessageType
{
    STATELESS,
    STATEFUL,
    COMMAND
};

/**
 * @brief Wrapper for Message data and type
 *
 */
class Message
{
public:
    MessageType type;
    nlohmann::json data;
    std::string moduleName;
    Message(MessageType t, nlohmann::json d, std::string mN = "")
        : type(t)
        , data(d)
        , moduleName(mN)
    {
    }
};

#endif // SHARED_H
