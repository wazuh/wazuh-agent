#pragma once

#include <agent_info.hpp>
#include <configuration_parser.hpp>
#include <http_client.hpp>

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
        /// @param name The agent's name.
        /// @param configFile The path to the configuration file.
        AgentRegistration(std::string user,
                          std::string password,
                          const std::string& key,
                          const std::string& name,
                          std::optional<std::string> configFile);

        /// @brief Registers the agent with the manager.
        ///
        /// @param httpClient The HTTP client to use for registration.
        /// @return True if the registration was successful, false otherwise.
        bool Register(http_client::IHttpClient& httpClient);

    private:
        configuration::ConfigurationParser m_configurationParser;
        std::string m_serverUrl;
        std::string m_registrationUrl;
        std::string m_user;
        std::string m_password;
        AgentInfo m_agentInfo;
    };
} // namespace agent_registration
