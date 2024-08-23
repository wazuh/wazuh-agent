#pragma once

#include <ihttp_client.hpp>

#include <string>

namespace registration
{
    struct UserCredentials
    {
        std::string user;
        std::string password;
    };

    bool RegisterAgent(const UserCredentials& userCredentials, http_client::IHttpClient& httpClient);

} // namespace registration
