#include <register.hpp>

#include <agent_info.hpp>
#include <configuration_parser.hpp>
#include <http_client.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;

namespace
{
    http::status SendRegistrationRequest(const std::string& managerIp,
                                         const std::string& port,
                                         const std::string& token,
                                         const std::string& uuid,
                                         const std::optional<std::string>& name,
                                         const std::optional<std::string>& ip)
    {
        json bodyJson = {{"uuid", uuid}};

        if (name.has_value())
        {
            bodyJson["name"] = name.value();
        }

        if (ip.has_value())
        {
            bodyJson["ip"] = ip.value();
        }

        const auto reqParams =
            http_client::HttpRequestParams(http::verb::post, managerIp, port, "/agents", token, "", bodyJson.dump());
        const http::response<http::dynamic_body> res = http_client::SendHttpRequest(reqParams);
        return res.result();
    }
} // namespace

namespace registration
{
    bool RegisterAgent(const UserCredentials& userCredentials)
    {
        const configuration::ConfigurationParser configurationParser;
        const auto managerIp = configurationParser.GetConfig<std::string>("agent", "manager_ip");
        const auto port = configurationParser.GetConfig<std::string>("agent", "port");

        const auto token =
            http_client::AuthenticateWithUserPassword(managerIp, port, userCredentials.user, userCredentials.password);

        if (!token.has_value())
        {
            std::cerr << "Failed to authenticate with the manager" << std::endl;
            return false;
        }

        const AgentInfo agentInfo {};

        if (const auto registrationResultCode = SendRegistrationRequest(
                managerIp, port, token.value(), agentInfo.GetUUID(), agentInfo.GetName(), agentInfo.GetIP());
            registrationResultCode != http::status::ok)
        {
            std::cout << "Registration error: " << registrationResultCode << std::endl;
            return false;
        }

        return true;
    }

} // namespace registration
