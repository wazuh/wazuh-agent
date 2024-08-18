#include <register.hpp>

#include <agent_info.hpp>
#include <configuration_parser.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/version.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;

namespace registration
{
    http::status SendRegistrationRequest(const std::string& host,
                                         const std::string& port,
                                         const std::string& token,
                                         const std::string& uuid,
                                         const std::string& key,
                                         const std::optional<std::string>& name)
    {
        json bodyJson = {{"uuid", uuid}, {"key", key}};

        if (name.has_value())
        {
            bodyJson["name"] = name.value();
        }

        const auto reqParams =
            http_client::HttpRequestParams(http::verb::post, host, port, "/agents", token, "", bodyJson.dump());
        const http::response<http::dynamic_body> res = http_client::PerformHttpRequest(reqParams);
        return res.result();
    }

    bool RegisterAgent(const UserCredentials& userCredentials,
                       std::function<http_client::AuthenticateFunctionType> AuthenticateFunction,
                       std::function<RegisterFunctionType> RegistrationFunction)
    {
        const configuration::ConfigurationParser configurationParser;
        const auto managerIp = configurationParser.GetConfig<std::string>("agent", "manager_ip");
        const auto managerPort = configurationParser.GetConfig<std::string>("agent", "server_mgmt_api_port");

        const auto token = AuthenticateFunction(managerIp, managerPort, userCredentials.user, userCredentials.password);

        if (!token.has_value())
        {
            std::cerr << "Failed to authenticate with the manager" << std::endl;
            return false;
        }

        const AgentInfo agentInfo {};

        if (const auto registrationResultCode = RegistrationFunction(
                managerIp, managerPort, token.value(), agentInfo.GetUUID(), agentInfo.GetKey(), agentInfo.GetName());
            registrationResultCode != http::status::ok)
        {
            std::cout << "Registration error: " << registrationResultCode << std::endl;
            return false;
        }

        return true;
    }

} // namespace registration
