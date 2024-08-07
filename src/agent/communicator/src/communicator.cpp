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

namespace
{
    boost::asio::awaitable<void> MessageProcessingTask(const std::string host,
                                                       const std::string port,
                                                       const std::string target,
                                                       const std::string& token,
                                                       std::queue<std::string>& messageQueue)
    {
        using namespace std::chrono_literals;

        auto executor = co_await boost::asio::this_coro::executor;
        boost::asio::steady_timer timer(executor);
        boost::asio::ip::tcp::resolver resolver(executor);

        while (true)
        {
            boost::beast::error_code ec;
            boost::asio::ip::tcp::socket socket(executor);

            auto const results = co_await resolver.async_resolve(host, port, boost::asio::use_awaitable);
            co_await boost::asio::async_connect(socket, results, boost::asio::use_awaitable);

            while (true)
            {
                std::string message;
                {
                    if (messageQueue.empty())
                    {
                        timer.expires_after(100ms);
                        co_await timer.async_wait(boost::asio::use_awaitable);
                        continue;
                    }
                    message = std::move(messageQueue.front());
                    messageQueue.pop();
                }

                auto req = http_client::CreateHttpRequest(boost::beast::http::verb::post, target, host, token, message);

                co_await http_client::Co_PerformHttpRequest(socket, req, ec);

                if (ec)
                {
                    socket.close();
                    break;
                }
            }
        }
    }
} // namespace

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
        using namespace std::chrono_literals;

        auto executor = co_await boost::asio::this_coro::executor;
        boost::asio::steady_timer timer(executor);
        boost::asio::ip::tcp::resolver resolver(executor);

        std::string url = "/commands";

        while (true)
        {
            boost::asio::ip::tcp::socket socket(executor);

            auto const results = co_await resolver.async_resolve(m_managerIp, m_port, boost::asio::use_awaitable);

            boost::system::error_code code;
            co_await boost::asio::async_connect(
                socket, results, boost::asio::redirect_error(boost::asio::use_awaitable, code));

            if (code != boost::system::errc::success)
            {
                std::cerr << "Connect failed: " << code.message() << std::endl;
                socket.close();
                std::this_thread::sleep_for(1000ms);
                continue;
            }

            auto req = http_client::CreateHttpRequest(http::verb::get, url, m_managerIp, m_token);

            boost::beast::error_code ec;
            co_await http_client::Co_PerformHttpRequest(socket, req, ec);

            if (ec)
            {
                socket.close();
                continue;
            }

            auto duration = std::chrono::milliseconds(1000);
            timer.expires_after(duration);
            co_await timer.async_wait(boost::asio::use_awaitable);
        }
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
        co_await MessageProcessingTask(m_managerIp, m_port, "/stateful", m_token, messageQueue);
    }

    boost::asio::awaitable<void> Communicator::StatelessMessageProcessingTask(std::queue<std::string>& messageQueue)
    {
        co_await MessageProcessingTask(m_managerIp, m_port, "/stateless", m_token, messageQueue);
    }
} // namespace communicator
