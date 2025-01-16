#pragma once

#include <nlohmann/json.hpp>
#include <memory>
#include <string>

class StatelessEvent {
protected:
    std::string operation;
    std::string created;
    nlohmann::json data;

public:
    StatelessEvent(std::string op, std::string time, nlohmann::json d);
    virtual nlohmann::json generate() const = 0;
    virtual ~StatelessEvent() = default;
};

class NetworkEvent  : public StatelessEvent { public: using StatelessEvent::StatelessEvent; nlohmann::json generate() const override; };
class PackageEvent  : public StatelessEvent { public: using StatelessEvent::StatelessEvent; nlohmann::json generate() const override; };
class HotfixEvent   : public StatelessEvent { public: using StatelessEvent::StatelessEvent; nlohmann::json generate() const override; };
class PortEvent     : public StatelessEvent { public: using StatelessEvent::StatelessEvent; nlohmann::json generate() const override; };
class ProcessEvent  : public StatelessEvent { public: using StatelessEvent::StatelessEvent; nlohmann::json generate() const override; };
class SystemEvent   : public StatelessEvent { public: using StatelessEvent::StatelessEvent; nlohmann::json generate() const override; };
class HardwareEvent : public StatelessEvent { public: using StatelessEvent::StatelessEvent; nlohmann::json generate() const override; };

std::unique_ptr<StatelessEvent> CreateStatelessEvent(const std::string& type, const std::string& operation, const std::string& created, const nlohmann::json& data);
