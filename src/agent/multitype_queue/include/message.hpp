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
 * @brief Wrapper for Message, contains the message type, the json data, the module name and the module type.
 *
 */
class Message
{
public:
    MessageType type;
    nlohmann::json data;
    std::string moduleName;
    std::string moduleType;

    Message(MessageType t, nlohmann::json d, std::string mN = "", std::string mT = "")
        : type(t)
        , data(d)
        , moduleName(mN)
        , moduleType(mT)
    {
    }

    // Define equality operator
    bool operator==(const Message& other) const
    {
        return type == other.type && data == other.data;
    }
};
