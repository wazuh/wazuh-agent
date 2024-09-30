#include <http_client.hpp>

#include "http_resolver_factory.hpp"
#include "http_socket_factory.hpp"

#include <logger.hpp>

#include <boost/beast/core/detail/base64.hpp>
#include <nlohmann/json.hpp>

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
    HttpClient::Co_PerformHttpRequest(std::shared_ptr<std::string> token,
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
            auto socket = m_socketFactory->Create(executor, reqParams.Use_Https);

            const auto results = co_await resolver->AsyncResolve(reqParams.Host, reqParams.Port);

            boost::system::error_code code;
            co_await socket->AsyncConnect(results, code);

            if (code != boost::system::errc::success)
            {
                LogError("Connect failed: {}.", code.message());
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

            reqParams.Token = *token;
            auto req = CreateHttpRequest(reqParams);

            boost::beast::error_code ec;
            co_await socket->AsyncWrite(req, ec);

            if (ec)
            {
                LogError("Error writing request ({}): {}.", std::to_string(ec.value()), ec.message());
                socket->Close();
                co_return;
            }

            boost::beast::http::response<boost::beast::http::dynamic_body> res;
            co_await socket->AsyncRead(res, ec);

            if (ec)
            {
                LogError("Error reading response. Response code: {}.", res.result_int());
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
            else if (res.result() == boost::beast::http::status::unauthorized ||
                     res.result() == boost::beast::http::status::forbidden)
            {
                if (onUnauthorized != nullptr)
                {
                    onUnauthorized();
                }
            }

            LogDebug("Response code: {}.", res.result_int());
            LogDebug("Response body: {}.", boost::beast::buffers_to_string(res.body().data()));

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

            auto socket = m_socketFactory->Create(io_context.get_executor(), params.Use_Https);
            socket->Connect(results);

            const auto req = CreateHttpRequest(params);
            socket->Write(req);
            socket->Read(res);

            LogDebug("Response code: {}.", res.result_int());
            LogDebug("Response body: {}.", boost::beast::buffers_to_string(res.body().data()));
        }
        catch (std::exception const& e)
        {
            LogError("Error: {}.", e.what());
            res.result(boost::beast::http::status::internal_server_error);
            boost::beast::ostream(res.body()) << "Internal server error: " << e.what();
            res.prepare_payload();
        }

        return res;
    }

    std::optional<std::string> HttpClient::AuthenticateWithUuidAndKey(const std::string& host,
                                                                      const std::string& port,
                                                                      const std::string& uuid,
                                                                      const std::string& key,
                                                                      const bool useHttps)
    {
        const std::string body = R"({"uuid":")" + uuid + R"(", "key":")" + key + "\"}";
        const auto reqParams = http_client::HttpRequestParams(
            boost::beast::http::verb::post, host, port, "/api/v1/authentication", useHttps, "", "", body);

        const auto res = PerformHttpRequest(reqParams);

        if (res.result() != boost::beast::http::status::ok)
        {
            LogError("Error: {}.", res.result_int());
            return std::nullopt;
        }

        return nlohmann::json::parse(boost::beast::buffers_to_string(res.body().data()))
            .at("token")
            .get_ref<const std::string&>();
    }

    std::optional<std::string> HttpClient::AuthenticateWithUserPassword(const std::string& host,
                                                                        const std::string& port,
                                                                        const std::string& user,
                                                                        const std::string& password,
                                                                        const bool useHttps)
    {
        std::string basicAuth {};
        std::string userPass {user + ":" + password};

        basicAuth.resize(boost::beast::detail::base64::encoded_size(userPass.size()));

        boost::beast::detail::base64::encode(&basicAuth[0], userPass.c_str(), userPass.size());

        const auto reqParams = http_client::HttpRequestParams(
            boost::beast::http::verb::post, host, port, "/security/user/authenticate", useHttps, "", basicAuth);

        const auto res = PerformHttpRequest(reqParams);

        if (res.result() != boost::beast::http::status::ok)
        {
            LogError("Error: {}.", res.result_int());
            return std::nullopt;
        }

        return nlohmann::json::parse(boost::beast::buffers_to_string(res.body().data()))
            .at("data")
            .at("token")
            .get_ref<const std::string&>();
    }
} // namespace http_client
