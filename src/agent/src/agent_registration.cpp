#include <agent_registration.hpp>

#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/verb.hpp>

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
                                         const std::string& dbFolderPath,
                                         std::string verificationMode)
        : m_httpClient(std::move(httpClient))
        , m_agentInfo(
              dbFolderPath, [this]() { return m_sysInfo.os(); }, [this]() { return m_sysInfo.networks(); }, true)
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

        const auto reqParams = http_client::HttpRequestParams(boost::beast::http::verb::post,
                                                              m_serverUrl,
                                                              "/agents",
                                                              m_agentInfo.GetHeaderInfo(),
                                                              m_verificationMode,
                                                              token.value(),
                                                              "",
                                                              m_agentInfo.GetMetadataInfo());

        const auto res = m_httpClient->PerformHttpRequest(reqParams);

        if (res.result() != boost::beast::http::status::created)
        {
            std::cout << "Registration error: " << res.result_int() << ".\n";
            return false;
        }

        m_agentInfo.Save();
        return true;
    }

    std::optional<std::string> AgentRegistration::AuthenticateWithUserPassword()
    {
        std::string basicAuth {};
        std::string userPass {m_user + ":" + m_password};

        basicAuth.resize(boost::beast::detail::base64::encoded_size(userPass.size()));

        boost::beast::detail::base64::encode(&basicAuth[0], userPass.c_str(), userPass.size());

        const auto reqParams = http_client::HttpRequestParams(boost::beast::http::verb::post,
                                                              m_serverUrl,
                                                              "/security/user/authenticate",
                                                              m_agentInfo.GetHeaderInfo(),
                                                              m_verificationMode,
                                                              "",
                                                              basicAuth);

        const auto res = m_httpClient->PerformHttpRequest(reqParams);

        if (res.result() < boost::beast::http::status::ok ||
            res.result() >= boost::beast::http::status::multiple_choices)
        {
            std::cout << "Error authenticating: " << res.result_int() << ".\n";
            return std::nullopt;
        }

        try
        {
            return nlohmann::json::parse(boost::beast::buffers_to_string(res.body().data()))
                .at("data")
                .at("token")
                .get_ref<const std::string&>();
        }
        catch (const std::exception& e)
        {
            std::cout << "Error parsing token in response: " << e.what() << ".\n";
        }

        return std::nullopt;
    }
} // namespace agent_registration
