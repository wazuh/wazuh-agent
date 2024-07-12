#include <map>

#include <nlohmann/json.hpp>

//TODO: should be moved to Config
constexpr char DEFAULT_FILE_PATH[] = "/home/vagrant/FILE_";
constexpr char DEFAULT_DB_PATH[] = "queue.db";

/**
 * @brief Types of messages enum
 *
 */
enum MessageType
{
    STATE_LESS,
    STATE_FULL,
    COMMAND
};

/**
 * @brief Map for transforing Message type name to string
 *
 */
std::map<MessageType, std::string> MessageTypeName {
    {MessageType::STATE_LESS, "STATE_LESS"},
    {MessageType::STATE_FULL, "STATE_FULL"},
    {MessageType::COMMAND, "COMMAND"},
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
    Message(MessageType t, nlohmann::json d)
        : type(t)
        , data(d)
    {
    }

    /**
     * @brief overloading == operator for messages
     *
     * @param other Secondary Message parameter
     * @return true when both messages matches in type and data
     * @return false when the messages don't match in type and data
     */
    bool operator==(const Message& other) const
    {
        return (this->type == other.type) && (this->data == other.data);
    }
};
