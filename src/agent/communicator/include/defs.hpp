#pragma once

#include <string_view>

namespace communicator
{
    const std::string_view uuidKey = "uuid";
    const std::string_view tokenKey = "token";
    const std::string_view eventKey = "event";
    const std::string_view eventsKey = "events";
    const std::string_view passwordKey = "password";
    const std::string_view nameKey = "name";
    const std::string_view ipKey = "ip";
    const std::string_view bearerPrefix = "Bearer ";

    const std::string_view kURL = "http://localhost:8080";
    const std::string_view kUUID = "agent_uuid";
    const std::string_view kPASSWORD = "123456";
    const std::string_view kNAME = "agent_name";
    const std::string_view kIP = "agent_ip";
} // namespace communicator
