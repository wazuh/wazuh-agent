#include <register.hpp>

#include <agent_info.hpp>
#include <configuration_parser.hpp>
#include <http_client.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace uuids = boost::uuids;
using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;

namespace
{
    std::pair<http::status, std::string>
    SendAuthenticationRequest(const std::string& managerIp, const std::string& port, const std::string& user_pass)
    {
        const http::response<http::dynamic_body> res =
            http_client::SendRequest(managerIp, port, http::verb::post, "/authenticate", "", "", user_pass);
        const auto token = beast::buffers_to_string(res.body().data());

        return {res.result(), token};
    }

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

        const http::response<http::dynamic_body> res =
            http_client::SendRequest(managerIp, port, http::verb::post, "/agents", token, bodyJson.dump(), "");
        return res.result();
    }

    AgentInfo GenerateAgentInfo(const std::optional<std::string>& name, const std::optional<std::string>& ip)
    {
        AgentInfo agentInfo;
        if (agentInfo.GetUUID().empty())
        {
            agentInfo.SetUUID(to_string(uuids::random_generator()()));
        }

        if (name.has_value())
        {
            agentInfo.SetName(name.value());
        }

        if (ip.has_value())
        {
            agentInfo.SetIP(ip.value());
        }

        return agentInfo;
    }

} // namespace

namespace registration
{
    bool RegisterAgent(const std::string& user,
                       const std::string& password,
                       const std::optional<std::string>& name,
                       const std::optional<std::string>& ip)
    {

        const configuration::ConfigurationParser configurationParser;
        const auto managerIp = configurationParser.GetConfig<std::string>("agent", "manager_ip");
        const auto port = configurationParser.GetConfig<std::string>("agent", "port");

        const auto [authResultCode, token] = SendAuthenticationRequest(managerIp, port, user + ":" + password);

        if (authResultCode != http::status::ok)
        {
            std::cout << "Authentication error: " << authResultCode << std::endl;
            return false;
        }

        const auto agentInfo = GenerateAgentInfo(name, ip);

        if (const auto registrationResultCode = SendRegistrationRequest(
                managerIp, port, token, agentInfo.GetUUID(), agentInfo.GetName(), agentInfo.GetIP());
            registrationResultCode != http::status::ok)
        {
            std::cout << "Registration error: " << registrationResultCode << std::endl;
            return false;
        }

        return true;
    }

} // namespace registration
