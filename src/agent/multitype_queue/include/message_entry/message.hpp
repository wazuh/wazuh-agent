#pragma once

#include <nlohmann/json.hpp>

#include <string>

/// @brief Types of messages enum
enum class MessageType
{
    STATELESS,
    STATEFUL,
    COMMAND
};

/// @brief Wrapper for Message, contains the message type, the json data, the
/// module name, the module type and the metadata.
class Message
{
public:
    MessageType type;
    nlohmann::json data;
    std::string moduleName;
    std::string moduleType;
    std::string metaData;

    Message(MessageType t, nlohmann::json d, std::string mN = "", std::string mT = "", std::string mD = "")
        : type(t)
        , data(d)
        , moduleName(mN)
        , moduleType(mT)
        , metaData(mD)
    {
    }

    // Define equality operator
    bool operator==(const Message& other) const
    {
        return type == other.type && data == other.data && moduleName == other.moduleName &&
               moduleType == other.moduleType && metaData == other.metaData;
    }
};
