#include <communicator.hpp>
#include <config.h>
#include <http_request_params.hpp>

#include <boost/asio.hpp>

#include <nlohmann/json.hpp>

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>

#include <boost/url.hpp>

#include <logger.hpp>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <thread>
#include <utility>

namespace
{
    constexpr auto MIN_BATCH_SIZE = 1000ULL;
    constexpr auto MAX_BATCH_SIZE = 100000000ULL;

    boost::asio::awaitable<void> WaitForTimer(std::shared_ptr<boost::asio::steady_timer> timer,
                                              const std::time_t retryInMillis)
    {
        if (!timer)
        {
            LogError("Timer is null.");
            co_return;
        }
        const auto duration = std::chrono::milliseconds(retryInMillis);
        (*timer).expires_after(duration);
        co_await timer->async_wait(boost::asio::use_awaitable);
    }
} // namespace

namespace communicator
{
    constexpr int TOKEN_PRE_EXPIRY_SECS = 2;
    constexpr int A_SECOND_IN_MILLIS = 1000;
    // 10 seconds minimum command request timeout
    constexpr time_t COMMANDS_REQUEST_TIMEOUT_MAX = static_cast<time_t>(10) * A_SECOND_IN_MILLIS;
    // 15 minutes maximum command request timeout
    constexpr time_t COMMANDS_REQUEST_TIMEOUT_MIN = static_cast<time_t>(15) * 60 * A_SECOND_IN_MILLIS;

    Communicator::Communicator(std::unique_ptr<http_client::IHttpClient> httpClient,
                               std::shared_ptr<configuration::ConfigurationParser> configurationParser,
                               std::string uuid,
                               std::string key,
                               std::function<std::string()> getHeaderInfo)
        : m_httpClient(std::move(httpClient))
        , m_uuid(std::move(uuid))
        , m_key(std::move(key))
        , m_getHeaderInfo(std::move(getHeaderInfo))
        , m_token(std::make_shared<std::string>())
    {
        if (!m_httpClient)
        {
            throw std::runtime_error(std::string("Invalid HTTP Client passed."));
        }

        if (!configurationParser)
        {
            throw std::runtime_error(std::string("Invalid Configuration Parser passed."));
        }

        m_serverUrl = configurationParser->GetConfigOrDefault(config::agent::DEFAULT_SERVER_URL, "agent", "server_url");

        if (const boost::urls::url_view url(m_serverUrl); url.scheme() != "https")
        {
            LogInfo("Using insecure connection.");
        }

        m_retryInterval = configurationParser->GetTimeConfigOrDefault(
            config::agent::DEFAULT_RETRY_INTERVAL, "agent", "retry_interval");

        m_batchSize = configurationParser->GetBytesConfigInRangeOrDefault(
            config::agent::DEFAULT_BATCH_SIZE, MIN_BATCH_SIZE, MAX_BATCH_SIZE, "events", "batch_size");

        m_verificationMode = configurationParser->GetConfigOrDefault(
            config::agent::DEFAULT_VERIFICATION_MODE, "agent", "verification_mode");

        if (std::find(std::begin(config::agent::VALID_VERIFICATION_MODES),
                      std::end(config::agent::VALID_VERIFICATION_MODES),
                      m_verificationMode) == std::end(config::agent::VALID_VERIFICATION_MODES))
        {
            LogWarn("Incorrect value for 'verification_mode', in case of HTTPS connections the default value '{}' "
                    "is used.",
                    config::agent::DEFAULT_VERIFICATION_MODE);
            m_verificationMode = config::agent::DEFAULT_VERIFICATION_MODE;
        }

        m_timeoutCommands =
            configurationParser->GetTimeConfigInRangeOrDefault(config::agent::DEFAULT_COMMANDS_REQUEST_TIMEOUT,
                                                               COMMANDS_REQUEST_TIMEOUT_MIN,
                                                               COMMANDS_REQUEST_TIMEOUT_MAX,
                                                               "agent",
                                                               "commands_request_timeout");
    }

    bool Communicator::SendAuthenticationRequest()
    {
        const auto token = AuthenticateWithUuidAndKey();

        if (token.has_value())
        {
            *m_token = token.value();
            LogInfo("Successfully authenticated with the manager.");
        }
        else
        {
            LogWarn("Failed to authenticate with the manager. Retrying in {} seconds.",
                    m_retryInterval / A_SECOND_IN_MILLIS);
            return false;
        }

        try
        {
            const auto decoded = jwt::decode<jwt::traits::nlohmann_json>(*m_token);

            if (!decoded.has_payload_claim("exp"))
            {
                throw std::runtime_error("Token does not contain an 'exp' claim.");
            }

            const auto exp_claim = decoded.get_payload_claim("exp");
            const auto exp_time = exp_claim.as_date();
            m_tokenExpTimeInSeconds =
                std::chrono::duration_cast<std::chrono::seconds>(exp_time.time_since_epoch()).count();
        }
        catch (const std::exception& e)
        {
            LogError("Failed to decode token: {}", e.what());
            m_token->clear();
            m_tokenExpTimeInSeconds = 1;
            return false;
        }

        return true;
    }

    std::optional<std::string> Communicator::AuthenticateWithUuidAndKey()
    {
        const std::string body = R"({"uuid":")" + m_uuid + R"(", "key":")" + m_key + "\"}";
        const auto reqParams = http_client::HttpRequestParams(http_client::MethodType::POST,
                                                              m_serverUrl,
                                                              "/api/v1/authentication",
                                                              m_getHeaderInfo ? m_getHeaderInfo() : "",
                                                              m_verificationMode,
                                                              "",
                                                              "",
                                                              body);

        const auto res = m_httpClient->PerformHttpRequest(reqParams);
        const auto res_status = std::get<0>(res);
        const auto res_message = std::get<1>(res);

        if (res_status < http_client::HTTP_CODE_OK || res_status >= http_client::HTTP_CODE_MULTIPLE_CHOICES)
        {
            if (res_status == http_client::HTTP_CODE_UNAUTHORIZED || res_status == http_client::HTTP_CODE_FORBIDDEN)
            {
                std::string message {};

                try
                {
                    message = nlohmann::json::parse(res_message).at("message").get_ref<const std::string&>();
                }
                catch (const std::exception& e)
                {
                    LogError("Error parsing message in response: {}.", e.what());
                }

                if (message == "Invalid key" || message == "Agent does not exist")
                {
                    throw std::runtime_error(message);
                }
            }
            LogWarn("Error: {}.", res_status);
            return std::nullopt;
        }

        try
        {
            return nlohmann::json::parse(res_message).at("token").get_ref<const std::string&>();
        }
        catch (const std::exception& e)
        {
            LogError("Error parsing token in response: {}.", e.what());
        }

        return std::nullopt;
    }

    long Communicator::GetTokenRemainingSecs() const
    {
        const auto now = std::chrono::system_clock::now();
        const auto now_seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        return std::max(0L, static_cast<long>(m_tokenExpTimeInSeconds - now_seconds));
    }

    boost::asio::awaitable<void> Communicator::WaitForTokenExpirationAndAuthenticate()
    {
        using namespace std::chrono_literals;
        const auto executor = co_await boost::asio::this_coro::executor;
        m_tokenExpTimer = std::make_unique<boost::asio::steady_timer>(executor);

        if (auto remainingSecs = GetTokenRemainingSecs(); remainingSecs > TOKEN_PRE_EXPIRY_SECS)
        {
            m_tokenExpTimer->expires_after(
                std::chrono::milliseconds((remainingSecs - TOKEN_PRE_EXPIRY_SECS) * A_SECOND_IN_MILLIS));

            boost::system::error_code ec;
            co_await m_tokenExpTimer->async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        }

        while (m_keepRunning.load())
        {
            const auto duration = [this]()
            {
                try
                {
                    if (!SendAuthenticationRequest())
                    {
                        return std::chrono::milliseconds(m_retryInterval);
                    }
                    else
                    {
                        return std::chrono::milliseconds((GetTokenRemainingSecs() - TOKEN_PRE_EXPIRY_SECS) *
                                                         A_SECOND_IN_MILLIS);
                    }
                }
                catch (const std::exception&)
                {
                    return std::chrono::milliseconds(m_retryInterval);
                }
            }();

            m_tokenExpTimer->expires_after(duration);

            boost::system::error_code ec;
            co_await m_tokenExpTimer->async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));

            if (ec)
            {
                if (ec == boost::asio::error::operation_aborted)
                {
                    LogDebug("Token expiration timer was canceled.");
                }
                else
                {
                    LogDebug("Timer wait failed: {}.", ec.message());
                }
            }
        }
    }

    boost::asio::awaitable<void>
    Communicator::GetCommandsFromManager(std::function<void(const int, const std::string&)> onSuccess)
    {
        const auto reqParams = http_client::HttpRequestParams(http_client::MethodType::GET,
                                                              m_serverUrl,
                                                              "/api/v1/commands",
                                                              m_getHeaderInfo ? m_getHeaderInfo() : "",
                                                              m_verificationMode,
                                                              "",
                                                              "",
                                                              "",
                                                              m_timeoutCommands);
        co_await ExecuteRequestLoop(reqParams, {}, onSuccess);
    }

    boost::asio::awaitable<void> Communicator::StatefulMessageProcessingTask(
        std::function<boost::asio::awaitable<std::tuple<int, std::string>>(const size_t)> getMessages,
        std::function<void(const int, const std::string&)> onSuccess)
    {
        const auto reqParams = http_client::HttpRequestParams(http_client::MethodType::POST,
                                                              m_serverUrl,
                                                              "/api/v1/events/stateful",
                                                              m_getHeaderInfo ? m_getHeaderInfo() : "",
                                                              m_verificationMode);
        co_await ExecuteRequestLoop(reqParams, getMessages, onSuccess);
    }

    boost::asio::awaitable<void> Communicator::StatelessMessageProcessingTask(
        std::function<boost::asio::awaitable<std::tuple<int, std::string>>(const size_t)> getMessages,
        std::function<void(const int, const std::string&)> onSuccess)
    {
        const auto reqParams = http_client::HttpRequestParams(http_client::MethodType::POST,
                                                              m_serverUrl,
                                                              "/api/v1/events/stateless",
                                                              m_getHeaderInfo ? m_getHeaderInfo() : "",
                                                              m_verificationMode);
        co_await ExecuteRequestLoop(reqParams, getMessages, onSuccess);
    }

    void Communicator::TryReAuthenticate()
    {
        const std::unique_lock<std::mutex> lock(m_reAuthMutex, std::try_to_lock);
        if (lock.owns_lock() && !m_isReAuthenticating.exchange(true))
        {
            if (m_tokenExpTimer)
            {
                m_tokenExpTimer->cancel();
            }
            m_isReAuthenticating = false;
        }
        else
        {
            const std::thread::id threadId = std::this_thread::get_id();
            std::ostringstream oss;
            oss << threadId;
            std::string threadIdStr = oss.str();
            LogDebug("Re-authentication attempt by thread {} failed.", threadIdStr);
        }
    }

    boost::asio::awaitable<bool> Communicator::GetGroupConfigurationFromManager(std::string groupName,
                                                                                std::string dstFilePath)
    {
        bool downloaded = false;

        if (m_token && !m_token->empty())
        {
            const auto reqParams = http_client::HttpRequestParams(http_client::MethodType::GET,
                                                                  m_serverUrl,
                                                                  "/api/v1/files?file_name=" + groupName +
                                                                      config::DEFAULT_SHARED_FILE_EXTENSION,
                                                                  m_getHeaderInfo ? m_getHeaderInfo() : "",
                                                                  m_verificationMode,
                                                                  *m_token);

            const auto res = co_await m_httpClient->Co_PerformHttpRequest(reqParams);
            const auto res_status = std::get<0>(res);
            const auto res_message = std::get<1>(res);

            if (res_status >= http_client::HTTP_CODE_OK && res_status < http_client::HTTP_CODE_MULTIPLE_CHOICES)
            {
                std::ofstream file(dstFilePath, std::ios::binary);
                if (file)
                {
                    file << res_message;
                    file.close();
                    downloaded = true;
                }
            }
            else
            {
                if (res_status == http_client::HTTP_CODE_UNAUTHORIZED || res_status == http_client::HTTP_CODE_FORBIDDEN)
                {
                    TryReAuthenticate();
                }
            }
        }

        co_return downloaded;
    }

    boost::asio::awaitable<void> Communicator::ExecuteRequestLoop(
        http_client::HttpRequestParams reqParams,
        std::function<boost::asio::awaitable<std::tuple<int, std::string>>(const size_t)> messageGetter,
        std::function<void(const int, const std::string&)> onSuccess)
    {
        using namespace std::chrono_literals;

        auto executor = co_await boost::asio::this_coro::executor;
        auto timer = std::make_shared<boost::asio::steady_timer>(executor);

        do
        {
            if (!m_token || m_token->empty())
            {
                co_await WaitForTimer(timer, A_SECOND_IN_MILLIS);
                continue;
            }

            auto messagesCount = 0;

            if (messageGetter != nullptr)
            {
                while (m_keepRunning.load())
                {
                    const auto messages = co_await messageGetter(m_batchSize);
                    messagesCount = std::get<0>(messages);

                    if (messagesCount)
                    {
                        LogTrace("Items count: {}", messagesCount);
                        reqParams.Body = std::get<1>(messages);
                        break;
                    }
                }
            }
            else
            {
                reqParams.Body = "";
            }

            reqParams.Token = *m_token;

            const auto res = co_await m_httpClient->Co_PerformHttpRequest(reqParams);
            const auto res_status = std::get<0>(res);
            const auto res_message = std::get<1>(res);

            std::time_t timerSleep = A_SECOND_IN_MILLIS;

            if (res_status >= http_client::HTTP_CODE_OK && res_status < http_client::HTTP_CODE_MULTIPLE_CHOICES)
            {
                if (onSuccess != nullptr)
                {
                    onSuccess(messagesCount, res_message);
                }
            }
            else
            {
                if (res_status == http_client::HTTP_CODE_UNAUTHORIZED || res_status == http_client::HTTP_CODE_FORBIDDEN)
                {
                    TryReAuthenticate();
                }
                if (res_status != http_client::HTTP_CODE_TIMEOUT)
                {
                    timerSleep = m_retryInterval;
                }
            }

            co_await WaitForTimer(timer, timerSleep);
        } while (m_keepRunning.load());
    }

    void Communicator::Stop()
    {
        m_keepRunning.store(false);
    }
} // namespace communicator
