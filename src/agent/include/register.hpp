#pragma once

#include <http_client.hpp>

#include <boost/beast/http.hpp>
#include <functional>
#include <optional>
#include <string>

namespace registration
{
    struct UserCredentials
    {
        std::string user;
        std::string password;
    };

    using RegisterFunctionType = boost::beast::http::status(const std::string& host,
                                                            const std::string& port,
                                                            const std::string& token,
                                                            const std::string& uuid,
                                                            const std::string& key,
                                                            const std::optional<std::string>& name);

    RegisterFunctionType SendRegistrationRequest;

    bool RegisterAgent(const UserCredentials& userCredentials,
                       std::function<http_client::AuthenticateFunctionType> AuthenticateFunction,
                       std::function<RegisterFunctionType> RegisterFunction);

} // namespace registration
