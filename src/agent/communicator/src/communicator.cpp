#include <communicator.hpp>

#include <http_client.hpp>

#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/version.hpp>
#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <queue>
#include <thread>

using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;

namespace communicator
{
    constexpr int TokenPreExpirySecs = 2;

    Communicator::Communicator(const std::string& uuid,
                               const std::function<std::string(std::string, std::string)> GetStringConfigValue)
        : m_exitFlag(false)
        , m_tokenExpTimeInSeconds(0)
        , m_uuid(uuid)
    {
        if (GetStringConfigValue != nullptr)
        {
            m_managerIp = GetStringConfigValue("agent", "manager_ip");
            m_port = GetStringConfigValue("agent", "port");
        }
    }

    Communicator::~Communicator()
    {
        Stop();
    }

    http::status Communicator::SendAuthenticationRequest()
    {
        json bodyJson = {{"uuid", m_uuid}};

        const auto res = http_client::SendHttpRequest(
            boost::beast::http::verb::post, m_managerIp, m_port, "/authentication", "", bodyJson.dump());

        if (res.result() != http::status::ok)
        {
            return res.result();
        }

        m_token = beast::buffers_to_string(res.body().data());

        auto decoded = jwt::decode(m_token);
        // Extract the expiration time claim (exp)
        if (decoded.has_payload_claim("exp"))
        {
            auto exp_claim = decoded.get_payload_claim("exp");
            auto exp_time = exp_claim.as_date();
            m_tokenExpTimeInSeconds =
                std::chrono::duration_cast<std::chrono::seconds>(exp_time.time_since_epoch()).count();
        }
        else
        {
            std::cerr << "Token does not contain an 'exp' claim" << std::endl;
        }

        return res.result();
    }

    const long Communicator::GetTokenRemainingSecs() const
    {
        auto now = std::chrono::system_clock::now();
        auto now_seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

        return std::max(0L, static_cast<long>(m_tokenExpTimeInSeconds - now_seconds));
    }

    void Communicator::Stop()
    {
        std::lock_guard<std::mutex> lock(m_exitMtx);
        m_exitFlag = true;
    }

    boost::asio::awaitable<void> Communicator::GetCommandsFromManager()
    {
        co_await http_client::Co_MessageProcessingTask(
            boost::beast::http::verb::get, m_managerIp, m_port, "/commands", m_token, {});
    }

    boost::asio::awaitable<void> Communicator::WaitForTokenExpirationAndAuthenticate()
    {
        using namespace std::chrono_literals;
        auto executor = co_await boost::asio::this_coro::executor;
        boost::asio::steady_timer timer(executor);

        while (true)
        {
            http::status result = SendAuthenticationRequest();

            auto duration = std::chrono::milliseconds(1000);
            if (result != http::status::ok)
            {
                std::cerr << "Authentication failed." << std::endl;
            }
            else
            {
                duration = std::chrono::milliseconds((GetTokenRemainingSecs() - TokenPreExpirySecs) * 1000);
            }

            timer.expires_after(duration);
            co_await timer.async_wait(boost::asio::use_awaitable);

            {
                std::lock_guard<std::mutex> lock(m_exitMtx);
                if (m_exitFlag)
                    co_return;
            }
        }
    }

    boost::asio::awaitable<void> Communicator::StatefulMessageProcessingTask(std::queue<std::string>& messageQueue)
    {
        co_await http_client::Co_MessageProcessingTask(
            boost::beast::http::verb::post, m_managerIp, m_port, "/stateful", m_token, {});
    }

    boost::asio::awaitable<void> Communicator::StatelessMessageProcessingTask(std::queue<std::string>& messageQueue)
    {
        co_await http_client::Co_MessageProcessingTask(
            boost::beast::http::verb::post, m_managerIp, m_port, "/stateless", m_token, {});
    }
} // namespace communicator
