#pragma once

#include <agent_info.hpp>
#include <configuration_parser.hpp>
#include <ihttp_client.hpp>

#include <sysInfo.hpp>

#include <optional>
#include <string>

namespace agent_registration
{
    /// @struct UserCredentials
    /// @brief Stores the user's credentials.
    struct UserCredentials
    {
        std::string user;
        std::string password;
    };

    /// @brief Registers an agent with a manager.
    ///
    /// This class provides methods for registering the agent and storing its
    /// configuration.
    class AgentRegistration
    {
    public:
        ///@brief Constructor for the AgentRegistration class.
        ///
        /// @param user The user's username.
        /// @param password The user's password.
        /// @param key The agent's key.
        /// @param configFile The path to the configuration file.
        AgentRegistration(std::string user,
                          std::string password,
                          const std::string& key,
                          std::optional<std::string> configFile);

        /// @brief Registers the agent with the manager.
        ///
        /// @param httpClient The HTTP client to use for registration.
        /// @return True if the registration was successful, false otherwise.
        bool Register(http_client::IHttpClient& httpClient);

    private:
        /// @brief The system's information.
        SysInfo m_sysInfo;

        /// @brief The agent's information.
        AgentInfo m_agentInfo;

        /// @brief The configuration parser.
        configuration::ConfigurationParser m_configurationParser;

        /// @brief The URL of the manager.
        std::string m_serverUrl;

        /// @brief The user's username.
        std::string m_user;

        /// @brief The user's password.
        std::string m_password;
    };
} // namespace agent_registration
