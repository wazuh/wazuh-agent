#include "communicator.hpp"
#include "defs.hpp"
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/version.hpp>
#include <iostream>
#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>

using tcp = boost::asio::ip::tcp;
using json = nlohmann::json;

namespace communicator
{
    constexpr int TokenPreExpirySecs = 2;

    Communicator::Communicator(const std::function<std::string(std::string, std::string)> GetStringConfigValue)
        : m_exitFlag(false)
        , m_tokenExpTimeInSeconds(0)
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

    const std::string& Communicator::GetToken() const
    {
        return m_token;
    }

    int Communicator::SendAuthenticationRequest()
    {
        json bodyJson = {{communicator::uuidKey, communicator::kUUID},
                         {communicator::passwordKey, communicator::kPASSWORD}};
        http::response<http::dynamic_body> res =
            sendHttpRequest(http::verb::post, "/authentication", "", bodyJson.dump());
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

        return res.result_int();
    }

    int Communicator::SendRegistrationRequest()
    {
        // TO DO: Create uuid to send to the server.
        json bodyJson = {{communicator::uuidKey, communicator::kUUID},
                         {communicator::nameKey, communicator::kNAME},
                         {communicator::ipKey, communicator::kIP}};
        http::response<http::dynamic_body> res = sendHttpRequest(http::verb::post, "/agents", m_token, bodyJson.dump());
        return res.result_int();
    }

    http::response<http::dynamic_body> Communicator::sendHttpRequest(http::verb method,
                                                                     const std::string& url,
                                                                     const std::string& token,
                                                                     const std::string& body)
    {
        http::response<http::dynamic_body> res;
        try
        {
            boost::asio::io_context io_context;
            tcp::resolver resolver(io_context);
            auto const results = resolver.resolve(m_managerIp, m_port);

            tcp::socket socket(io_context);
            boost::asio::connect(socket, results.begin(), results.end());

            http::request<http::string_body> req {method, url, 11};
            req.set(http::field::host, "localhost");
            req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            req.set(http::field::accept, "application/json");

            if (!token.empty())
            {
                req.set(http::field::authorization, "Bearer " + token);
            }

            if (!body.empty())
            {
                req.set(http::field::content_type, "application/json");
                req.body() = body;
                req.prepare_payload();
            }

            http::write(socket, req);

            beast::flat_buffer buffer;

            http::read(socket, buffer, res);

            std::cout << "Response code: " << res.result_int() << std::endl;
            std::cout << "Response body: " << beast::buffers_to_string(res.body().data()) << std::endl;
        }
        catch (std::exception const& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            res.result(http::status::internal_server_error);
            beast::ostream(res.body()) << "Internal server error: " << e.what();
            res.prepare_payload();
            std::cerr << "Result: " << res.result_int() << std::endl;
        }
        return res;
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

    boost::asio::awaitable<void> Communicator::WaitForTokenExpirationAndAuthenticate()
    {
        using namespace std::chrono_literals;
        auto executor = co_await boost::asio::this_coro::executor;
        boost::asio::steady_timer timer(executor);

        while (true)
        {
            SendAuthenticationRequest();
            auto duration = std::chrono::milliseconds((GetTokenRemainingSecs() - TokenPreExpirySecs) * 1000);
            timer.expires_after(duration);
            co_await timer.async_wait(boost::asio::use_awaitable);

            {
                std::lock_guard<std::mutex> lock(m_exitMtx);
                if (m_exitFlag)
                    co_return;
            }
        }
    }
} // namespace communicator
