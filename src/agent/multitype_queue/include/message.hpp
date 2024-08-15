#pragma once

#include <nlohmann/json.hpp>

#include <string>

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
};
