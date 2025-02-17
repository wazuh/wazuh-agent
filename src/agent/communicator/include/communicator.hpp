#pragma once

#include <configuration_parser.hpp>
#include <ihttp_client.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/steady_timer.hpp>

#include <atomic>
#include <ctime>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

namespace communicator
{
    /// @brief Communicator class
    ///
    /// This class handles communication with the server, manages authentication,
    /// and processes messages. It provides methods for authentication, command
    /// retrieval, and message processing, both stateful and stateless. It also
    /// allows for stopping the communication process.
    class Communicator
    {
    public:
        /// @brief Communicator constructor
        /// @tparam ConfigGetter Type of the configuration getter function
        /// @param httpClient The HTTP client to use for communication
        /// @param configurationParser The configuration parser
        /// @param uuid The unique identifier for the agent
        /// @param key The key for authentication
        /// @param getHeaderInfo Function to get the user agent header
        Communicator(std::unique_ptr<http_client::IHttpClient> httpClient,
                     std::shared_ptr<configuration::ConfigurationParser> configurationParser,
                     std::string uuid,
                     std::string key,
                     std::function<std::string()> getHeaderInfo);

        /// @brief Sends an authentication request to the manager
        /// @return true if the request was sent successfully, false otherwise
        bool SendAuthenticationRequest();

        /// @brief Authenticate with a UUID and key
        /// @return The authentication token
        std::optional<std::string> AuthenticateWithUuidAndKey();

        /// @brief Waits for the authentication token to expire and authenticates again
        boost::asio::awaitable<void> WaitForTokenExpirationAndAuthenticate();

        /// @brief Retrieves commands from the manager
        /// @param onSuccess A callback function to execute when a command is received
        boost::asio::awaitable<void>
        GetCommandsFromManager(std::function<void(const int, const std::string&)> onSuccess);

        /// @brief Processes messages in a stateful manner
        /// @param getMessages A function to retrieve a message from the queue
        /// @param onSuccess A callback function to execute when a message is processed
        boost::asio::awaitable<void> StatefulMessageProcessingTask(
            std::function<boost::asio::awaitable<std::tuple<int, std::string>>(const size_t)> getMessages,
            std::function<void(const int, const std::string&)> onSuccess);

        /// @brief Processes messages in a stateless manner
        /// @param getMessages A function to retrieve a message from the queue
        /// @param onSuccess A callback function to execute when a message is processed
        boost::asio::awaitable<void> StatelessMessageProcessingTask(
            std::function<boost::asio::awaitable<std::tuple<int, std::string>>(const size_t)> getMessages,
            std::function<void(const int, const std::string&)> onSuccess);

        /// @brief Retrieves group configuration from the manager
        /// @param groupName The name of the group to retrieve the configuration for
        /// @param dstFilePath The path to the file to store the configuration in
        /// @return true if the configuration was successfully retrieved, false otherwise
        boost::asio::awaitable<bool> GetGroupConfigurationFromManager(std::string groupName, std::string dstFilePath);

        /// @brief Sends a message to the manager signaling the agent has started
        /// @details This method sends both stateful and stateless messages
        void SendAgentStartupMessage();

        /// @brief Sends a message to the manager signaling the agent has shut down
        /// @details This method sends both stateful and stateless messages
        void SendAgentShutdownMessage();

        /// @brief Stops the communication process
        void Stop();

    private:
        /// @brief Calculates the remaining time (in seconds) until the authentication token expires
        /// @return The remaining time in seconds until the authentication token expires
        long GetTokenRemainingSecs() const;

        /// @brief Checks if the authentication token has expired and authenticates again if necessary
        void TryReAuthenticate();

        /// @brief Executes a request loop
        /// @param reqParams The parameters for the request
        /// @param messageGetter Function to retrieve messages
        /// @param onSuccess Action to take on successful request
        boost::asio::awaitable<void> ExecuteRequestLoop(
            http_client::HttpRequestParams reqParams,
            std::function<boost::asio::awaitable<std::tuple<int, std::string>>(const size_t)> messageGetter = {},
            std::function<void(const int, const std::string&)> onSuccess = {});

        /// @brief Indicates if the communication process should keep running
        std::atomic<bool> m_keepRunning = true;

        /// @brief The HTTP client to use for communication
        std::unique_ptr<http_client::IHttpClient> m_httpClient;

        /// @brief Mutex to protect authentication attempts
        std::mutex m_reAuthMutex;

        /// @brief Indicates if an authentication attempt is currently in progress
        std::atomic<bool> m_isReAuthenticating = false;

        /// @brief Time in milliseconds between authentication attemps in case of failure
        std::time_t m_retryInterval;

        /// @brief Size for batch requests
        size_t m_batchSize;

        /// @brief The server URL
        std::string m_serverUrl;

        /// @brief The agent's unique identifier
        std::string m_uuid;

        /// @brief The agent's key
        std::string m_key;

        /// @brief The user agent header
        std::function<std::string()> m_getHeaderInfo;

        /// @brief The authentication token
        std::shared_ptr<std::string> m_token;

        /// @brief The time (in seconds) until the authentication token expires
        long long m_tokenExpTimeInSeconds = 0;

        /// @brief Timer to wait for token expiration
        std::unique_ptr<boost::asio::steady_timer> m_tokenExpTimer;

        /// @brief The verification mode
        std::string m_verificationMode;

        /// @brief Timeout for command requests to manager in millisecconds.
        std::time_t m_timeoutCommands;
    };
} // namespace communicator
