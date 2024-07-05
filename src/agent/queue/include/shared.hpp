#include <map>

#include <nlohmann/json.hpp>

constexpr char DEFAULT_PERS_PATH[] = "/home/vagrant/FILE_";

/**
 * @brief 
 * 
 */
enum  MessageType {
    STATE_LESS,
    STATE_FULL,
    COMMAND
};

/**
 * @brief 
 * 
 */
std::map<MessageType, std::string> MessageTypeName {
    {MessageType::STATE_LESS, "STATE_LESS"},
    {MessageType::STATE_FULL, "STATE_FULL"},
    {MessageType::COMMAND, "COMMAND"},
};


/**
 * @brief 
 * 
 */
class Message {
public:
    MessageType type;
    nlohmann::json data;
    Message(MessageType t, nlohmann::json d) : type(t), data(d) {}
};
