#include <communicator.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>

#include <algorithm>
#include <chrono>
#include <sstream>
#include <thread>
#include <utility>

namespace communicator
{
    constexpr int TOKEN_PRE_EXPIRY_SECS = 2;
    constexpr int A_SECOND_IN_MILLIS = 1000;

    boost::beast::http::status Communicator::SendAuthenticationRequest()
    {
        const auto token = m_httpClient->AuthenticateWithUuidAndKey(
            m_serverUrl, m_getHeaderInfo ? m_getHeaderInfo() : "", m_uuid, m_key);

        if (token.has_value())
        {
            *m_token = token.value();
            LogInfo("Successfully authenticated with the manager.");
        }
        else
        {
            LogWarn("Failed to authenticate with the manager. Retrying in {} seconds.",
                    m_retryInterval / A_SECOND_IN_MILLIS);
            return boost::beast::http::status::unauthorized;
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
            boost::beast::http::verb::get, m_serverUrl, "/api/v1/commands", m_getHeaderInfo ? m_getHeaderInfo() : "");
        co_await m_httpClient->Co_PerformHttpRequest(
            m_token, reqParams, {}, onAuthenticationFailed, m_retryInterval, onSuccess, loopCondition);
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
                    return std::chrono::milliseconds(m_retryInterval * A_SECOND_IN_MILLIS);
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

        const auto reqParams = http_client::HttpRequestParams(boost::beast::http::verb::post,
                                                              m_serverUrl,
                                                              "/api/v1/events/stateful",
                                                              m_getHeaderInfo ? m_getHeaderInfo() : "");
        co_await m_httpClient->Co_PerformHttpRequest(
            m_token, reqParams, getMessages, onAuthenticationFailed, m_retryInterval, onSuccess, loopCondition);
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

        const auto reqParams = http_client::HttpRequestParams(boost::beast::http::verb::post,
                                                              m_serverUrl,
                                                              "/api/v1/events/stateless",
                                                              m_getHeaderInfo ? m_getHeaderInfo() : "");
        co_await m_httpClient->Co_PerformHttpRequest(
            m_token, reqParams, getMessages, onAuthenticationFailed, m_retryInterval, onSuccess, loopCondition);
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
                                                              m_serverUrl,
                                                              "/api/v1/files?file_name=" + groupName + ".conf",
                                                              m_getHeaderInfo ? m_getHeaderInfo() : "",
                                                              *m_token);

        const auto result = m_httpClient->PerformHttpRequestDownload(reqParams, dstFilePath);

        return result.result() == boost::beast::http::status::ok;
    }

    void Communicator::Stop()
    {
        m_keepRunning.store(false);
    }
} // namespace communicator
