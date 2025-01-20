#include <agent_registration.hpp>

#include <boost/beast/http.hpp>
#include <config.h>
#include <nlohmann/json.hpp>

#include <iostream>

namespace http = boost::beast::http;

namespace agent_registration
{
    AgentRegistration::AgentRegistration(std::string url,
                                         std::string user,
                                         std::string password,
                                         const std::string& key,
                                         const std::string& name,
                                         const std::string& dbFolderPath,
                                         std::string verificationMode)
        : m_agentInfo(
              dbFolderPath, [this]() { return m_sysInfo.os(); }, [this]() { return m_sysInfo.networks(); }, true)
        , m_serverUrl(std::move(url))
        , m_user(std::move(user))
        , m_password(std::move(password))
        , m_verificationMode(std::move(verificationMode))
    {
        if (!m_agentInfo.SetKey(key))
        {
            throw std::invalid_argument("--key argument must be alphanumeric and 32 characters in length");
        }

        if (!m_agentInfo.SetName(name))
        {
            throw std::runtime_error("Couldn't set agent name");
        }
    }

    bool AgentRegistration::Register(http_client::IHttpClient& httpClient)
    {
        const auto token = httpClient.AuthenticateWithUserPassword(
            m_serverUrl, m_agentInfo.GetHeaderInfo(), m_user, m_password, m_verificationMode);

        if (!token.has_value())
        {
            std::cout << "Failed to authenticate with the manager\n";
            return false;
        }

        const auto reqParams = http_client::HttpRequestParams(http::verb::post,
                                                              m_serverUrl,
                                                              "/agents",
                                                              m_agentInfo.GetHeaderInfo(),
                                                              m_verificationMode,
                                                              token.value(),
                                                              "",
                                                              m_agentInfo.GetMetadataInfo());

        const auto res = httpClient.PerformHttpRequest(reqParams);

        if (res.result() != http::status::created)
        {
            std::cout << "Registration error: " << res.result_int() << ".\n";
            return false;
        }

        m_agentInfo.Save();
        return true;
    }

} // namespace agent_registration
