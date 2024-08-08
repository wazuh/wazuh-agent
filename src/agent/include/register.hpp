#pragma once

#include <optional>
#include <string>

namespace registration
{
    struct UserCredentials
    {
        std::string user;
        std::string password;
    };

    struct AgentInfoOptionalData
    {
        std::optional<std::string> name;
        std::optional<std::string> ip;
    };

    bool RegisterAgent(const UserCredentials& userCredentials, const AgentInfoOptionalData& agentInfoOptionalData);
} // namespace registration
