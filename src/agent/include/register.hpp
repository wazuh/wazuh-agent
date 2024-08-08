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

    bool RegisterAgent(const UserCredentials& userCredentials);
} // namespace registration
