#include <register.hpp>

#include <agent_info.hpp>
#include <configuration_parser.hpp>

#include <boost/beast/http.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

namespace http = boost::beast::http;

namespace registration
{
    bool RegisterAgent(const UserCredentials& userCredentials, http_client::IHttpClient& httpClient)
    {
        const configuration::ConfigurationParser configurationParser;
        const auto managerIp = configurationParser.GetConfig<std::string>("agent", "manager_ip");
        const auto managerPort = configurationParser.GetConfig<std::string>("agent", "server_mgmt_api_port");

        const auto token = httpClient.AuthenticateWithUserPassword(
            managerIp, managerPort, userCredentials.user, userCredentials.password);

        if (!token.has_value())
        {
            std::cerr << "Failed to authenticate with the manager" << '\n';
            return false;
        }

        const AgentInfo agentInfo {};

        nlohmann::json bodyJson = {{"uuid", agentInfo.GetUUID()}, {"key", agentInfo.GetKey()}};

        if (!agentInfo.GetName().empty())
        {
            bodyJson["name"] = agentInfo.GetName();
        }

        const auto reqParams = http_client::HttpRequestParams(
            http::verb::post, managerIp, managerPort, "/agents", token.value(), "", bodyJson.dump());

        const auto res = httpClient.PerformHttpRequest(reqParams);

        if (res.result() != http::status::ok)
        {
            std::cout << "Registration error: " << res.result() << '\n';
            return false;
        }

        return true;
    }

} // namespace registration
