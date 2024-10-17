#include <communicator.hpp>

#include <logger.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

#include <jwt-cpp/jwt.h>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#include <algorithm>
#include <chrono>
#include <queue>
#include <thread>
#include <utility>

namespace communicator
{
    constexpr int TOKEN_PRE_EXPIRY_SECS = 2;
    constexpr int A_SECOND_IN_MILLIS = 1000;

    Communicator::Communicator(std::unique_ptr<http_client::IHttpClient> httpClient,
                               std::string uuid,
                               std::string key,
                               const std::function<std::string(std::string, std::string)>& getStringConfigValue)
        : m_httpClient(std::move(httpClient))
        , m_uuid(std::move(uuid))
        , m_key(std::move(key))
        , m_token(std::make_shared<std::string>())
    {
        if (getStringConfigValue != nullptr)
        {
            m_managerIp = getStringConfigValue("agent", "manager_ip");
            m_port = getStringConfigValue("agent", "agent_comms_api_port");
            const std::string httpsEnabled = getStringConfigValue("agent", "https_enabled");
            if (httpsEnabled == "no")
            {
                m_useHttps = false;
                LogInfo("Using insecure connection.");
            }
        }
    }

    boost::beast::http::status Communicator::SendAuthenticationRequest()
    {
        const auto token = m_httpClient->AuthenticateWithUuidAndKey(m_managerIp, m_port, m_uuid, m_key, m_useHttps);

        if (token.has_value())
        {
            *m_token = token.value();
        }
        else
        {
            LogError("Failed to authenticate with the manager.");
            return boost::beast::http::status::unauthorized;
        }

        if (const auto decoded = jwt::decode(*m_token); decoded.has_payload_claim("exp"))
        {
            const auto exp_claim = decoded.get_payload_claim("exp");
            const auto exp_time = exp_claim.as_date();
            m_tokenExpTimeInSeconds =
                std::chrono::duration_cast<std::chrono::seconds>(exp_time.time_since_epoch()).count();
        }
        else
        {
            LogError("Token does not contain an 'exp' claim.");
            m_token->clear();
            m_tokenExpTimeInSeconds = 1;
            return boost::beast::http::status::unauthorized;
        }

        return boost::beast::http::status::ok;
    }

    long Communicator::GetTokenRemainingSecs() const
    {
        const auto now = std::chrono::system_clock::now();
        const auto now_seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        return std::max(0L, static_cast<long>(m_tokenExpTimeInSeconds - now_seconds));
    }

    boost::asio::awaitable<void> Communicator::GetCommandsFromManager(std::function<void(const std::string&)> onSuccess)
    {
        auto onAuthenticationFailed = [this]()
        {
            TryReAuthenticate();
        };

        auto loopCondition = [this]()
        {
            return m_keepRunning.load();
        };

        const auto reqParams = http_client::HttpRequestParams(
            boost::beast::http::verb::get, m_managerIp, m_port, "/api/v1/commands", m_useHttps);
        co_await m_httpClient->Co_PerformHttpRequest(
            m_token, reqParams, {}, onAuthenticationFailed, onSuccess, loopCondition);
    }

    boost::asio::awaitable<void> Communicator::WaitForTokenExpirationAndAuthenticate()
    {
        using namespace std::chrono_literals;
        const auto executor = co_await boost::asio::this_coro::executor;
        m_tokenExpTimer = std::make_unique<boost::asio::steady_timer>(executor);

        while (m_keepRunning.load())
        {
            const auto duration = [this]()
            {
                const auto result = SendAuthenticationRequest();
                if (result != boost::beast::http::status::ok)
                {
                    return std::chrono::milliseconds(A_SECOND_IN_MILLIS);
                }
                else
                {
                    return std::chrono::milliseconds((GetTokenRemainingSecs() - TOKEN_PRE_EXPIRY_SECS) *
                                                     A_SECOND_IN_MILLIS);
                }
            }();

            m_tokenExpTimer->expires_after(duration);

            boost::system::error_code ec;
            co_await m_tokenExpTimer->async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));

            if (ec)
            {
                if (ec == boost::asio::error::operation_aborted)
                {
                    LogError("Token expiration timer was canceled.");
                }
                else
                {
                    LogError("Timer wait failed: {}.", ec.message());
                }
            }
        }
    }

    boost::asio::awaitable<void>
    Communicator::StatefulMessageProcessingTask(std::function<boost::asio::awaitable<std::string>()> getMessages,
                                                std::function<void(const std::string&)> onSuccess)
    {
        auto onAuthenticationFailed = [this]()
        {
            TryReAuthenticate();
        };

        auto loopCondition = [this]()
        {
            return m_keepRunning.load();
        };

        const auto reqParams = http_client::HttpRequestParams(
            boost::beast::http::verb::post, m_managerIp, m_port, "/api/v1/events/stateful", m_useHttps);
        co_await m_httpClient->Co_PerformHttpRequest(
            m_token, reqParams, getMessages, onAuthenticationFailed, onSuccess, loopCondition);
    }

    boost::asio::awaitable<void>
    Communicator::StatelessMessageProcessingTask(std::function<boost::asio::awaitable<std::string>()> getMessages,
                                                 std::function<void(const std::string&)> onSuccess)
    {
        auto onAuthenticationFailed = [this]()
        {
            TryReAuthenticate();
        };

        auto loopCondition = [this]()
        {
            return m_keepRunning.load();
        };

        const auto reqParams = http_client::HttpRequestParams(
            boost::beast::http::verb::post, m_managerIp, m_port, "/api/v1/events/stateless", m_useHttps);
        co_await m_httpClient->Co_PerformHttpRequest(
            m_token, reqParams, getMessages, onAuthenticationFailed, onSuccess, loopCondition);
    }

    void Communicator::TryReAuthenticate()
    {
        std::unique_lock<std::mutex> lock(m_reAuthMutex, std::try_to_lock);
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
            LogError("Re-authentication attempt by thread {} failed.", threadIdStr);
        }
    }

    bool Communicator::GetGroupConfigurationFromManager(const std::string& groupName, const std::string& dstFilePath)
    {
        const auto reqParams = http_client::HttpRequestParams(boost::beast::http::verb::get,
                                                              m_managerIp,
                                                              m_port,
                                                              "/api/v1/files?file_name=" + groupName + ".conf",
                                                              m_useHttps,
                                                              *m_token,
                                                              "",
                                                              "");

        const auto result = m_httpClient->PerformHttpRequestDownload(reqParams, dstFilePath);

        return result.result() == boost::beast::http::status::ok;
    }

    void Communicator::Stop()
    {
        m_keepRunning.store(false);
    }
} // namespace communicator
