#pragma once

#include <ihttp_client.hpp>

#include <config.h>
#include <logger.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/url.hpp>

#include <atomic>
#include <ctime>
#include <functional>
#include <memory>
#include <mutex>
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
        /// @param uuid The unique identifier for the agent
        /// @param key The key for authentication
        /// @param getHeaderInfo Function to get the user agent header
        /// @param getConfigValue Function to retrieve configuration values
        template<typename ConfigGetter>
        Communicator(std::unique_ptr<http_client::IHttpClient> httpClient,
                     std::string uuid,
                     std::string key,
                     std::function<std::string()> getHeaderInfo,
                     const ConfigGetter& getConfigValue)
            : m_httpClient(std::move(httpClient))
            , m_uuid(std::move(uuid))
            , m_key(std::move(key))
            , m_getHeaderInfo(std::move(getHeaderInfo))
            , m_token(std::make_shared<std::string>())
        {
            m_serverUrl = getConfigValue.template operator()<std::string>("agent", "server_url")
                              .value_or(config::agent::DEFAULT_SERVER_URL);

            if (boost::urls::url_view url(m_serverUrl); url.scheme() != "https")
            {
                LogInfo("Using insecure connection.");
            }

            m_retryInterval = getConfigValue.template operator()<std::time_t>("agent", "retry_interval")
                                  .value_or(config::agent::DEFAULT_RETRY_INTERVAL);

            m_batchInterval = getConfigValue.template operator()<std::time_t>("events", "batch_interval")
                                  .value_or(config::agent::DEFAULT_BATCH_INTERVAL);

            if (m_batchInterval < 1'000 || m_batchInterval > (1'000 * 60 * 60))
            {
                LogWarn("batch_interval must be between 1s and 1h. Using default value.");
                m_batchInterval = config::agent::DEFAULT_BATCH_INTERVAL;
            }

            m_batchSize = getConfigValue.template operator()<int>("events", "batch_size")
                              .value_or(config::agent::DEFAULT_BATCH_SIZE);

            if (m_batchSize < 1'000 || m_batchSize > 1'000'000)
            {
                LogWarn("batch_size must be between 1000 and 1000000. Using default value.");
                m_batchSize = config::agent::DEFAULT_BATCH_SIZE;
            }
        }

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
            std::function<boost::asio::awaitable<std::tuple<int, std::string>>(const int)> getMessages,
            std::function<void(const int, const std::string&)> onSuccess);

        /// @brief Processes messages in a stateless manner
        /// @param getMessages A function to retrieve a message from the queue
        /// @param onSuccess A callback function to execute when a message is processed
        boost::asio::awaitable<void> StatelessMessageProcessingTask(
            std::function<boost::asio::awaitable<std::tuple<int, std::string>>(const int)> getMessages,
            std::function<void(const int, const std::string&)> onSuccess);

        /// @brief Retrieves group configuration from the manager
        /// @param groupName The name of the group to retrieve the configuration for
        /// @param dstFilePath The path to the file to store the configuration in
        /// @return true if the configuration was successfully retrieved, false otherwise
        bool GetGroupConfigurationFromManager(const std::string& groupName, const std::string& dstFilePath);

        /// @brief Stops the communication process
        void Stop();

    private:
        /// @brief Calculates the remaining time (in seconds) until the authentication token expires
        /// @return The remaining time in seconds until the authentication token expires
        long GetTokenRemainingSecs() const;

        /// @brief Sends an authentication request to the manager
        /// @return The HTTP status of the authentication request
        boost::beast::http::status SendAuthenticationRequest();

        /// @brief Checks if the authentication token has expired and authenticates again if necessary
        void TryReAuthenticate();

        /// @brief Indicates if the communication process should keep running
        std::atomic<bool> m_keepRunning = true;

        /// @brief The HTTP client to use for communication
        std::unique_ptr<http_client::IHttpClient> m_httpClient;

        /// @brief Mutex to protect authentication attempts
        std::mutex m_reAuthMutex;

        /// @brief Indicates if an authentication attempt is currently in progress
        std::atomic<bool> m_isReAuthenticating = false;

        /// @brief Time in milliseconds between authentication attemps in case of failure
        std::time_t m_retryInterval = config::agent::DEFAULT_RETRY_INTERVAL;

        /// @brief Time between batch requests
        std::time_t m_batchInterval = config::agent::DEFAULT_BATCH_INTERVAL;

        /// @brief Maximum number of messages to batch
        int m_batchSize = config::agent::DEFAULT_BATCH_SIZE;

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
    };
} // namespace communicator
