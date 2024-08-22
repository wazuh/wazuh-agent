#pragma once

#include <nlohmann/json.hpp>

#include <string>

/**
 * @brief Types of messages enum
 *
 */
enum class MessageType
{
    STATELESS,
    STATEFUL,
    COMMAND
};

/**
 * @brief Wrapper for Message, contains the message type, the json data and the module name.
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

    // Define equality operator
    bool operator==(const Message& other) const
    {
        return type == other.type && data == other.data;
    }
};
