#include <message_task.hpp>

#include <chrono>
#include <iostream>

namespace
{

    boost::asio::awaitable<void> send_http_request(const std::string host,
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

                // HTTP request
                boost::beast::http::request<boost::beast::http::string_body> req {
                    boost::beast::http::verb::post, target, 11};
                req.set(boost::beast::http::field::host, host);
                req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
                req.set(boost::beast::http::field::authorization, "Bearer " + token);
                req.body() = message;
                req.prepare_payload();
                co_await boost::beast::http::async_write(
                    socket, req, boost::asio::redirect_error(boost::asio::use_awaitable, ec));

                if (ec)
                {
                    socket.close();
                    break;
                }

                // HTTP response
                boost::beast::flat_buffer buffer;
                boost::beast::http::response<boost::beast::http::dynamic_body> res;
                co_await boost::beast::http::async_read(
                    socket, buffer, res, boost::asio::redirect_error(boost::asio::use_awaitable, ec));

                if (ec)
                {
                    socket.close();
                    break;
                }
            }
        }
    }

} // namespace

boost::asio::awaitable<void> StatefulMessageProcessingTask(const std::string& manager_ip, const std::string& port, const std::string& token,
                                                           std::queue<std::string>& messageQueue)
{
    co_await send_http_request(manager_ip, port, "/stateless", token, messageQueue);
}

boost::asio::awaitable<void> StatelessMessageProcessingTask(const std::string& manager_ip, const std::string& port, const std::string& token,
                                                            std::queue<std::string>& messageQueue)
{
    co_await send_http_request(manager_ip, port, "/stateful", token, messageQueue);
}
