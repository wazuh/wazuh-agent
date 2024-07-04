#include <nlohmann/json.hpp>

constexpr char DEFAULT_PERS_PATH[] = "/home/vagrant/FILE";


enum MessageType {
    STATE_LESS,
    STATE_FULL,
    COMMAND
};

class Message {
public:
    MessageType type;
    nlohmann::json data;
    Message(MessageType t, std::string d) : type(t), data(d) {}
};