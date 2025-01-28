#pragma once

#include <agent_info.hpp>
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
        /// @param httpClient The HTTP client to use for communication.
        /// @param url The server URL.
        /// @param user The user's username.
        /// @param password The user's password.
        /// @param key The agent's key.
        /// @param name The agent's name.
        /// @param dbFolderPath The path to the database folder.
        /// @param verificationMode The connection verification mode.
        AgentRegistration(std::unique_ptr<http_client::IHttpClient> httpClient,
                          std::string url,
                          std::string user,
                          std::string password,
                          const std::string& key,
                          const std::string& name,
                          const std::string& dbFolderPath,
                          std::string verificationMode);

        /// @brief Registers the agent with the manager.
        ///
        /// @return True if the registration was successful, false otherwise.
        bool Register();

        /// @brief Authenticates using username and password
        ///
        /// @return Authentication token if successful, otherwise nullopt
        std::optional<std::string> AuthenticateWithUserPassword();

    private:
        /// @brief The HTTP client to use for communication
        std::unique_ptr<http_client::IHttpClient> m_httpClient;

        /// @brief The system's information.
        SysInfo m_sysInfo;

        /// @brief The agent's information.
        AgentInfo m_agentInfo;

        /// @brief The URL of the manager.
        std::string m_serverUrl;

        /// @brief The user's username.
        std::string m_user;

        /// @brief The user's password.
        std::string m_password;

        /// @brief The connection verification mode.
        std::string m_verificationMode;
    };
} // namespace agent_registration
