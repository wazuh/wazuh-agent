#include <communicator.hpp>

#include <http_client.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <queue>
#include <thread>

namespace communicator
{
    constexpr int TokenPreExpirySecs = 2;

    Communicator::Communicator(const std::string& uuid,
                               const std::function<std::string(std::string, std::string)> GetStringConfigValue)
        : m_uuid(uuid)
    {
        if (GetStringConfigValue != nullptr)
        {
            m_managerIp = GetStringConfigValue("agent", "manager_ip");
            m_port = GetStringConfigValue("agent", "agent_comms_api_port");
        }
    }

    boost::beast::http::status Communicator::SendAuthenticationRequest()
    {
        const auto token = http_client::AuthenticateWithUuid(m_managerIp, m_port, m_uuid);

        if (token.has_value())
        {
            m_token = token.value();
        }
        else
        {
            std::cerr << "Failed to authenticate with the manager" << std::endl;
            return boost::beast::http::status::unauthorized;
        }

        if (const auto decoded = jwt::decode(m_token); decoded.has_payload_claim("exp"))
        {
            const auto exp_claim = decoded.get_payload_claim("exp");
            const auto exp_time = exp_claim.as_date();
            m_tokenExpTimeInSeconds =
                std::chrono::duration_cast<std::chrono::seconds>(exp_time.time_since_epoch()).count();
        }
        else
        {
            std::cerr << "Token does not contain an 'exp' claim" << std::endl;
            m_token.clear();
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

    boost::asio::awaitable<void> Communicator::GetCommandsFromManager()
    {
        auto onAuthenticationFailed = [this]()
        {
            TryReAuthenticate();
        };
        const auto reqParams =
            http_client::HttpRequestParams(boost::beast::http::verb::get, m_managerIp, m_port, "/commands");
        co_await http_client::Co_MessageProcessingTask(m_token, reqParams, {}, onAuthenticationFailed);
    }

    boost::asio::awaitable<void> Communicator::WaitForTokenExpirationAndAuthenticate()
    {
        using namespace std::chrono_literals;
        const auto executor = co_await boost::asio::this_coro::executor;
        m_tokenExpTimer = std::make_unique<boost::asio::steady_timer>(executor);

        while (true)
        {
            const auto duration = [this]()
            {
                const auto result = SendAuthenticationRequest();
                if (result != boost::beast::http::status::ok)
                {
                    std::cerr << "Authentication failed." << std::endl;
                    return std::chrono::milliseconds(1000);
                }
                else
                {
                    return std::chrono::milliseconds((GetTokenRemainingSecs() - TokenPreExpirySecs) * 1000);
                }
            }();

            m_tokenExpTimer->expires_after(duration);
            co_await m_tokenExpTimer->async_wait(boost::asio::use_awaitable);
        }
    }

    boost::asio::awaitable<void> Communicator::StatefulMessageProcessingTask(std::queue<std::string>& messageQueue)
    {
        auto onAuthenticationFailed = [this]()
        {
            TryReAuthenticate();
        };
        const auto reqParams =
            http_client::HttpRequestParams(boost::beast::http::verb::post, m_managerIp, m_port, "/stateful");
        co_await http_client::Co_MessageProcessingTask(m_token, reqParams, {}, onAuthenticationFailed);
    }

    boost::asio::awaitable<void> Communicator::StatelessMessageProcessingTask(std::queue<std::string>& messageQueue)
    {
        auto onAuthenticationFailed = [this]()
        {
            TryReAuthenticate();
        };
        const auto reqParams =
            http_client::HttpRequestParams(boost::beast::http::verb::post, m_managerIp, m_port, "/stateless");
        co_await http_client::Co_MessageProcessingTask(m_token, reqParams, {}, onAuthenticationFailed);
    }

    void Communicator::TryReAuthenticate()
    {
        std::unique_lock<std::mutex> lock(m_reAuthMutex, std::try_to_lock);
        if (lock.owns_lock() && !m_isReAuthenticating.exchange(true))
        {
            std::cout << "Thread: " << std::this_thread::get_id() << " attempting re-authentication" << std::endl;
            if (const auto result = SendAuthenticationRequest(); result == boost::beast::http::status::ok)
            {
                if (m_tokenExpTimer)
                {
                    const auto newDuration =
                        std::chrono::milliseconds((GetTokenRemainingSecs() - TokenPreExpirySecs) * 1000);
                    m_tokenExpTimer->expires_after(newDuration);
                }
            }
            m_isReAuthenticating = false;
        }
        else
        {
            std::cout << "Re-authentication attempt by thread " << std::this_thread::get_id() << " failed" << std::endl;
        }
    }
} // namespace communicator
