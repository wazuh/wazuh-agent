#include <agent_registration.hpp>
#include <http_request_params.hpp>

#include <config.h>
#include <nlohmann/json.hpp>

#include <iostream>

namespace agent_registration
{
    AgentRegistration::AgentRegistration(std::unique_ptr<http_client::IHttpClient> httpClient,
                                         std::string url,
                                         std::string user,
                                         std::string password,
                                         const std::string& key,
                                         const std::string& name,
                                         [[maybe_unused]] const std::string& dbFolderPath,
                                         std::string verificationMode,
                                         std::optional<AgentInfo> agentInfo)
        : m_httpClient(std::move(httpClient))
        , m_agentInfo(agentInfo.has_value() ? std::move(*agentInfo)
                                            : AgentInfo(
                                                  dbFolderPath,
                                                  [this]() { return m_sysInfo.os(); },
                                                  [this]() { return m_sysInfo.networks(); },
                                                  true))
        , m_serverUrl(std::move(url))
        , m_user(std::move(user))
        , m_password(std::move(password))
        , m_verificationMode(std::move(verificationMode))
    {
        if (!m_httpClient)
        {
            throw std::runtime_error("Invalid HTTP Client passed");
        }

        if (!m_agentInfo.SetKey(key))
        {
            throw std::invalid_argument("--key argument must be alphanumeric and 32 characters in length");
        }

        if (!m_agentInfo.SetName(name))
        {
            throw std::runtime_error("Couldn't set agent name");
        }
    }

    bool AgentRegistration::Register()
    {
        const auto token = AuthenticateWithUserPassword();

        if (!token.has_value())
        {
            std::cout << "Failed to authenticate with the manager\n";
            return false;
        }

        const auto reqParams = http_client::HttpRequestParams(http_client::MethodType::POST,
                                                              m_serverUrl,
                                                              "/agents",
                                                              m_agentInfo.GetHeaderInfo(),
                                                              m_verificationMode,
                                                              token.value(),
                                                              "",
                                                              m_agentInfo.GetMetadataInfo());

        const auto res = m_httpClient->PerformHttpRequest(reqParams);
        const auto res_status = std::get<0>(res);

        if (res_status != http_client::HTTP_CODE_CREATED)
        {
            std::cout << "Registration error: " << res_status << ".\n";
            return false;
        }

        m_agentInfo.Save();
        return true;
    }

    std::optional<std::string> AgentRegistration::AuthenticateWithUserPassword()
    {
        const auto reqParams = http_client::HttpRequestParams(http_client::MethodType::POST,
                                                              m_serverUrl,
                                                              "/security/user/authenticate",
                                                              m_agentInfo.GetHeaderInfo(),
                                                              m_verificationMode,
                                                              "",
                                                              m_user + ":" + m_password);

        const auto res = m_httpClient->PerformHttpRequest(reqParams);
        const auto res_status = std::get<0>(res);
        const auto res_message = std::get<1>(res);

        if (res_status < http_client::HTTP_CODE_OK || res_status >= http_client::HTTP_CODE_MULTIPLE_CHOICES)
        {
            std::cout << "Error authenticating: " << res_status << ".\n";
            return std::nullopt;
        }

        try
        {
            return nlohmann::json::parse(res_message).at("data").at("token").get_ref<const std::string&>();
        }
        catch (const std::exception& e)
        {
            std::cout << "Error parsing token in response: " << e.what() << ".\n";
        }

        return std::nullopt;
    }
} // namespace agent_registration
