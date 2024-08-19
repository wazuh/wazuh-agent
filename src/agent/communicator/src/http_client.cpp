#include <http_client.hpp>

#include <iostream>

namespace
{
    std::optional<std::string> GetTokenFromResponse(const http_client::HttpRequestParams& reqParams)
    {
        const auto res = http_client::PerformHttpRequest(reqParams);

        if (res.result() != boost::beast::http::status::ok)
        {
            std::cerr << "Error: " << res.result() << std::endl;
            return std::nullopt;
        }

        return boost::beast::buffers_to_string(res.body().data());
    }
} // namespace

namespace http_client
{
    boost::beast::http::request<boost::beast::http::string_body> CreateHttpRequest(const HttpRequestParams& params)
    {
        static constexpr int HttpVersion1_1 = 11;

        boost::beast::http::request<boost::beast::http::string_body> req {
            params.method, params.endpoint, HttpVersion1_1};
        req.set(boost::beast::http::field::host, params.host);
        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(boost::beast::http::field::accept, "application/json");

        if (!params.token.empty())
        {
            req.set(boost::beast::http::field::authorization, "Bearer " + params.token);
        }

        if (!params.user_pass.empty())
        {
            req.set(boost::beast::http::field::authorization, "Basic " + params.user_pass);
        }

        if (!params.body.empty())
        {
            req.set(boost::beast::http::field::content_type, "application/json");
            req.body() = params.body;
            req.prepare_payload();
        }

        return req;
    }

    boost::asio::awaitable<void>
    Co_PerformHttpRequest(boost::asio::ip::tcp::socket& socket,
                          boost::beast::http::request<boost::beast::http::string_body>& req,
                          boost::beast::error_code& ec,
                          std::function<void()> onUnauthorized,
                          std::function<void(const std::string&)> onSuccess)
    {
        co_await boost::beast::http::async_write(
            socket, req, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        if (ec)
        {
            std::cerr << "Error writing request (" << std::to_string(ec.value()) << "): " << ec.message() << std::endl;
            co_return;
        }

        boost::beast::flat_buffer buffer;
        boost::beast::http::response<boost::beast::http::dynamic_body> res;
        co_await boost::beast::http::async_read(
            socket, buffer, res, boost::asio::redirect_error(boost::asio::use_awaitable, ec));

        if (ec)
        {
            std::cerr << "Error reading response. Response code: " << res.result_int() << std::endl;
            co_return;
        }

        if (res.result_int() == static_cast<int>(boost::beast::http::status::unauthorized))
        {
            if (onUnauthorized != nullptr)
            {
                onUnauthorized();
            }
        }

        if (res.result_int() == static_cast<int>(boost::beast::http::status::ok))
        {
            if (onSuccess != nullptr)
            {
                onSuccess(boost::beast::buffers_to_string(res.body().data()));
            }
        }

        std::cout << "Response code: " << res.result_int() << std::endl;
        std::cout << "Response body: " << boost::beast::buffers_to_string(res.body().data()) << std::endl;
    }

    boost::asio::awaitable<void>
    Co_MessageProcessingTask(const std::string& token,
                             HttpRequestParams reqParams,
                             std::function<boost::asio::awaitable<std::string>()> messageGetter,
                             std::function<void()> onUnauthorized,
                             std::function<void(const std::string&)> onSuccess)
    {
        using namespace std::chrono_literals;

        auto executor = co_await boost::asio::this_coro::executor;
        boost::asio::steady_timer timer(executor);
        boost::asio::ip::tcp::resolver resolver(executor);

        while (true)
        {
            boost::asio::ip::tcp::socket socket(executor);

            const auto results =
                co_await resolver.async_resolve(reqParams.host, reqParams.port, boost::asio::use_awaitable);

            boost::system::error_code code;
            co_await boost::asio::async_connect(
                socket, results, boost::asio::redirect_error(boost::asio::use_awaitable, code));

            if (code != boost::system::errc::success)
            {
                std::cerr << "Connect failed: " << code.message() << std::endl;
                socket.close();
                const auto duration = std::chrono::milliseconds(1000);
                timer.expires_after(duration);
                co_await timer.async_wait(boost::asio::use_awaitable);
                continue;
            }

            if (messageGetter != nullptr)
            {
                reqParams.body = co_await messageGetter();
            }
            else
            {
                reqParams.body = "";
            }

            reqParams.token = token;
            auto req = CreateHttpRequest(reqParams);

            boost::beast::error_code ec;
            co_await Co_PerformHttpRequest(socket, req, ec, onUnauthorized, onSuccess);

            if (ec)
            {
                socket.close();
            }

            const auto duration = std::chrono::milliseconds(1000);
            timer.expires_after(duration);
            co_await timer.async_wait(boost::asio::use_awaitable);
        }
    }

    boost::beast::http::response<boost::beast::http::dynamic_body> PerformHttpRequest(const HttpRequestParams& params)
    {
        boost::beast::http::response<boost::beast::http::dynamic_body> res;

        try
        {
            boost::asio::io_context io_context;
            boost::asio::ip::tcp::resolver resolver(io_context);
            const auto results = resolver.resolve(params.host, params.port);

            boost::asio::ip::tcp::socket socket(io_context);
            boost::asio::connect(socket, results.begin(), results.end());

            const auto req = CreateHttpRequest(params);

            boost::beast::http::write(socket, req);

            boost::beast::flat_buffer buffer;

            boost::beast::http::read(socket, buffer, res);

            std::cout << "Response code: " << res.result_int() << std::endl;
            std::cout << "Response body: " << boost::beast::buffers_to_string(res.body().data()) << std::endl;
        }
        catch (std::exception const& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            res.result(boost::beast::http::status::internal_server_error);
            boost::beast::ostream(res.body()) << "Internal server error: " << e.what();
            res.prepare_payload();
        }

        return res;
    }

    std::optional<std::string> AuthenticateWithUuidAndKey(const std::string& host,
                                                          const std::string& port,
                                                          const std::string& uuid,
                                                          const std::string& key)
    {
        const std::string body = "{\"uuid\":\"" + uuid + "\", \"key\":\"" + key + "\"}";
        const auto reqParams =
            http_client::HttpRequestParams(boost::beast::http::verb::post, host, port, "/authentication", "", "", body);

        return GetTokenFromResponse(reqParams);
    }

    std::optional<std::string> AuthenticateWithUserPassword(const std::string& host,
                                                            const std::string& port,
                                                            const std::string& user,
                                                            const std::string& password)
    {
        const auto reqParams = http_client::HttpRequestParams(
            boost::beast::http::verb::post, host, port, "/authenticate", "", user + ":" + password);

        return GetTokenFromResponse(reqParams);
    }
} // namespace http_client
