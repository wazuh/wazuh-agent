#include <http_client.hpp>

#include "http_resolver_factory.hpp"
#include "http_socket_factory.hpp"

#include <iostream>

namespace
{
    std::optional<std::string>
    GetTokenFromResponse(const boost::beast::http::response<boost::beast::http::dynamic_body>& response)
    {
        if (response.result() != boost::beast::http::status::ok)
        {
            std::cerr << "Error: " << response.result() << std::endl;
            return std::nullopt;
        }

        return boost::beast::buffers_to_string(response.body().data());
    }
} // namespace

namespace http_client
{
    HttpClient::HttpClient(std::shared_ptr<IHttpResolverFactory> resolverFactory,
                           std::shared_ptr<IHttpSocketFactory> socketFactory)
    {
        if (resolverFactory != nullptr)
        {
            m_resolverFactory = std::move(resolverFactory);
        }
        else
        {
            m_resolverFactory = std::make_shared<HttpResolverFactory>();
        }

        if (socketFactory != nullptr)
        {
            m_socketFactory = std::move(socketFactory);
        }
        else
        {
            m_socketFactory = std::make_shared<HttpSocketFactory>();
        }
    }

    boost::beast::http::request<boost::beast::http::string_body>
    HttpClient::CreateHttpRequest(const HttpRequestParams& params)
    {
        static constexpr int HttpVersion1_1 = 11;

        boost::beast::http::request<boost::beast::http::string_body> req {
            params.Method, params.Endpoint, HttpVersion1_1};
        req.set(boost::beast::http::field::host, params.Host);
        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(boost::beast::http::field::accept, "application/json");

        if (!params.Token.empty())
        {
            req.set(boost::beast::http::field::authorization, "Bearer " + params.Token);
        }

        if (!params.User_pass.empty())
        {
            req.set(boost::beast::http::field::authorization, "Basic " + params.User_pass);
        }

        if (!params.Body.empty())
        {
            req.set(boost::beast::http::field::content_type, "application/json");
            req.body() = params.Body;
            req.prepare_payload();
        }

        return req;
    }

// Silence false positive warning introduced in newer versions of GCC
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#endif
    boost::asio::awaitable<void>
    HttpClient::Co_PerformHttpRequest(const std::string& token,
                                      HttpRequestParams reqParams,
                                      std::function<boost::asio::awaitable<std::string>()> messageGetter,
                                      std::function<void()> onUnauthorized,
                                      std::function<void(const std::string&)> onSuccess,
                                      std::function<bool()> loopRequestCondition)
    {
        using namespace std::chrono_literals;

        auto executor = co_await boost::asio::this_coro::executor;
        boost::asio::steady_timer timer(executor);
        auto resolver = m_resolverFactory->Create(executor);

        do
        {
            auto socket = m_socketFactory->Create(executor);

            const auto results = co_await resolver->AsyncResolve(reqParams.Host, reqParams.Port);

            boost::system::error_code code;
            co_await socket->AsyncConnect(results, code);

            if (code != boost::system::errc::success)
            {
                std::cerr << "Connect failed: " << code.message() << std::endl;
                socket->Close();
                const auto duration = std::chrono::milliseconds(1000);
                timer.expires_after(duration);
                co_await timer.async_wait(boost::asio::use_awaitable);
                continue;
            }

            if (messageGetter != nullptr)
            {
                reqParams.Body = co_await messageGetter();
            }
            else
            {
                reqParams.Body = "";
            }

            reqParams.Token = token;
            auto req = CreateHttpRequest(reqParams);

            boost::beast::error_code ec;
            co_await socket->AsyncWrite(req, ec);

            if (ec)
            {
                std::cerr << "Error writing request (" << std::to_string(ec.value()) << "): " << ec.message()
                          << std::endl;
                socket->Close();
                co_return;
            }

            boost::beast::http::response<boost::beast::http::dynamic_body> res;
            co_await socket->AsyncRead(res, ec);

            if (ec)
            {
                std::cerr << "Error reading response. Response code: " << res.result_int() << std::endl;
                socket->Close();
                co_return;
            }

            if (res.result() == boost::beast::http::status::ok)
            {
                if (onSuccess != nullptr)
                {
                    onSuccess(boost::beast::buffers_to_string(res.body().data()));
                }
            }
            else if (res.result() == boost::beast::http::status::unauthorized)
            {
                if (onUnauthorized != nullptr)
                {
                    onUnauthorized();
                }
            }

            std::cout << "Response code: " << res.result_int() << std::endl;
            std::cout << "Response body: " << boost::beast::buffers_to_string(res.body().data()) << std::endl;

            const auto duration = std::chrono::milliseconds(1000);
            timer.expires_after(duration);
            co_await timer.async_wait(boost::asio::use_awaitable);
        } while (loopRequestCondition != nullptr && loopRequestCondition());
    }
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

    boost::beast::http::response<boost::beast::http::dynamic_body>
    HttpClient::PerformHttpRequest(const HttpRequestParams& params)
    {
        boost::beast::http::response<boost::beast::http::dynamic_body> res;

        try
        {
            boost::asio::io_context io_context;
            auto resolver = m_resolverFactory->Create(io_context.get_executor());

            const auto results = resolver->Resolve(params.Host, params.Port);

            auto socket = m_socketFactory->Create(io_context.get_executor());
            socket->Connect(results);

            const auto req = CreateHttpRequest(params);
            socket->Write(req);
            socket->Read(res);

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

    std::optional<std::string> HttpClient::AuthenticateWithUuidAndKey(const std::string& host,
                                                                      const std::string& port,
                                                                      const std::string& uuid,
                                                                      const std::string& key)
    {
        const std::string body = "{\"uuid\":\"" + uuid + "\", \"key\":\"" + key + "\"}";
        const auto reqParams =
            http_client::HttpRequestParams(boost::beast::http::verb::post, host, port, "/authentication", "", "", body);

        const auto res = PerformHttpRequest(reqParams);
        return GetTokenFromResponse(res);
    }

    std::optional<std::string> HttpClient::AuthenticateWithUserPassword(const std::string& host,
                                                                        const std::string& port,
                                                                        const std::string& user,
                                                                        const std::string& password)
    {
        const auto reqParams = http_client::HttpRequestParams(
            boost::beast::http::verb::post, host, port, "/authenticate", "", user + ":" + password);

        const auto res = PerformHttpRequest(reqParams);
        return GetTokenFromResponse(res);
    }
} // namespace http_client
