#include <register.hpp>

#include <agent_info.hpp>
#include <configuration_parser.hpp>
#include <logger.hpp>

#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

namespace http = boost::beast::http;

namespace registration
{
    bool RegisterAgent(const UserCredentials& userCredentials, http_client::IHttpClient& httpClient)
    {
        const configuration::ConfigurationParser configurationParser;
        const auto managerIp = configurationParser.GetConfig<std::string>("agent", "manager_ip");
        const auto managerPort = configurationParser.GetConfig<std::string>("agent", "server_mgmt_api_port");
        const bool useHttps = !(configurationParser.GetConfig<std::string>("agent", "https_enabled") == "no");

        const auto token = httpClient.AuthenticateWithUserPassword(
            managerIp, managerPort, userCredentials.user, userCredentials.password, useHttps);

        if (!token.has_value())
        {
            LogError("Failed to authenticate with the manager");
            return false;
        }

        const AgentInfo agentInfo {};

        nlohmann::json bodyJson = {{"id", agentInfo.GetUUID()}, {"key", agentInfo.GetKey()}};

        if (!agentInfo.GetName().empty())
        {
            bodyJson["name"] = agentInfo.GetName();
        }

        const auto reqParams = http_client::HttpRequestParams(
            http::verb::post, managerIp, managerPort, "/agents", useHttps, token.value(), "", bodyJson.dump());

        const auto res = httpClient.PerformHttpRequest(reqParams);

        if (res.result() != http::status::ok)
        {
            LogError("Registration error: {}.", res.result_int());
            return false;
        }

        return true;
    }

} // namespace registration
