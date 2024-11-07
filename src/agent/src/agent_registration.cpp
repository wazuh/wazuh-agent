#include <agent_registration.hpp>

#include <boost/beast/http.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

namespace http = boost::beast::http;

namespace agent_registration
{
    AgentRegistration::AgentRegistration(std::string user,
                                         std::string password,
                                         const std::string& key,
                                         std::optional<std::string> configFile)
        : m_configurationParser(configFile.has_value() && !configFile->empty()
                                    ? configuration::ConfigurationParser(std::filesystem::path(configFile.value()))
                                    : configuration::ConfigurationParser())
        , m_serverUrl(m_configurationParser.GetConfig<std::string>("agent", "registration_url"))
        , m_user(std::move(user))
        , m_password(std::move(password))
    {
        if (!m_agentInfo.SetKey(key))
        {
            throw std::invalid_argument("--key argument must be alphanumeric and 32 characters in length");
        }
    }

    bool AgentRegistration::Register(http_client::IHttpClient& httpClient)
    {
        const auto token = httpClient.AuthenticateWithUserPassword(m_serverUrl,
                                                                   m_agentInfo.GetHeaderInfo(),
                                                                   m_user,
                                                                   m_password);

        if (!token.has_value())
        {
            std::cout << "Failed to authenticate with the manager\n";
            return false;
        }

        const auto reqParams = http_client::HttpRequestParams(http::verb::post,
                                                              m_serverUrl,
                                                              "/agents",
                                                              m_agentInfo.GetHeaderInfo(),
                                                              token.value(),
                                                              "",
                                                              m_agentInfo.GetMetadataInfo(true).dump());

        const auto res = httpClient.PerformHttpRequest(reqParams);

        if (res.result() != http::status::ok)
        {
            std::cout << "Registration error: " << res.result_int() << ".\n";
            return false;
        }

        return true;
    }

} // namespace agent_registration
