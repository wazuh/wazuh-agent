#include <http_client.hpp>

#include "http_resolver_factory.hpp"
#include "http_socket_factory.hpp"
#include "ihttp_socket.hpp"

#include <logger.hpp>

#include <boost/beast/core/detail/base64.hpp>
#include <nlohmann/json.hpp>

#include <chrono>
#include <sstream>
#include <string>

namespace
{
    boost::asio::awaitable<void> WaitForTimer(std::shared_ptr<boost::asio::steady_timer> timer,
                                              const std::time_t retryInMillis)
    {
        if (!timer)
        {
            LogError("Timer is null.");
            co_return;
        }
        const auto duration = std::chrono::milliseconds(retryInMillis);
        (*timer).expires_after(duration);
        co_await timer->async_wait(boost::asio::use_awaitable);
    }

    std::string ResponseToString(const std::string& endpoint,
                                 const boost::beast::http::response<boost::beast::http::dynamic_body>& res)
    {
        std::ostringstream stream;
        stream << "Request endpoint: " << endpoint << "\nResponse: " << res;
        return stream.str();
    }
} // namespace

namespace http_client
{
    constexpr int A_SECOND_IN_MILLIS = 1000;

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
        req.set(boost::beast::http::field::user_agent, params.User_agent);
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
            req.set(boost::beast::http::field::transfer_encoding, "chunked");
            req.body() = params.Body;
            req.prepare_payload();
        }

        return req;
    }

    boost::asio::awaitable<void> HttpClient::Co_PerformHttpRequest(
        std::shared_ptr<std::string> token,
        HttpRequestParams reqParams,
        std::function<boost::asio::awaitable<std::tuple<int, std::string>>(const size_t)> messageGetter,
        std::function<void()> onUnauthorized,
        std::time_t connectionRetry,
        size_t batchSize,
        std::function<void(const int, const std::string&)> onSuccess,
        std::function<bool()> loopRequestCondition)
    {
        using namespace std::chrono_literals;

        auto executor = co_await boost::asio::this_coro::executor;
        auto timer = std::make_shared<boost::asio::steady_timer>(executor);
        auto resolver = m_resolverFactory->Create(executor);

        do
        {
            if (!token || token->empty())
            {
                co_await WaitForTimer(timer, A_SECOND_IN_MILLIS);
                continue;
            }

            const auto results = co_await resolver->AsyncResolve(reqParams.Host, reqParams.Port);

            if (results.empty())
            {
                LogWarn("Failed to resolve host. Retrying in {} seconds.", connectionRetry / A_SECOND_IN_MILLIS);
                co_await WaitForTimer(timer, connectionRetry);
                continue;
            }

            auto socket = m_socketFactory->Create(executor, reqParams.Use_Https);

            if (!socket)
            {
                LogWarn("Failed to create socket. Retrying in {} seconds.", connectionRetry / A_SECOND_IN_MILLIS);
                co_await WaitForTimer(timer, connectionRetry);
                continue;
            }

            if (reqParams.Use_Https)
            {
                socket->SetVerificationMode(reqParams.Host, reqParams.Verification_Mode);
            }

            boost::system::error_code ec;

            co_await socket->AsyncConnect(results, ec);

            if (ec != boost::system::errc::success)
            {
                LogDebug("Failed to send http request to endpoint: {}. Retrying in {} seconds.",
                         reqParams.Endpoint,
                         connectionRetry / A_SECOND_IN_MILLIS);
                LogDebug("Http request failed: {} - {}", ec.message(), ec.what());
                co_await WaitForTimer(timer, connectionRetry);
                continue;
            }

            auto messagesCount = 0;

            if (messageGetter != nullptr)
            {
                while (loopRequestCondition != nullptr && loopRequestCondition())
                {
                    const auto messages = co_await messageGetter(batchSize);
                    messagesCount = std::get<0>(messages);

                    if (messagesCount)
                    {
                        LogTrace("Messages count: {}", messagesCount);
                        reqParams.Body = std::get<1>(messages);
                        break;
                    }
                }
            }
            else
            {
                reqParams.Body = "";
            }

            reqParams.Token = *token;

            const auto req = CreateHttpRequest(reqParams);

            co_await socket->AsyncWrite(req, ec);

            if (ec)
            {
                LogDebug("Error writing request ({}): {}. Endpoint: {}.",
                         std::to_string(ec.value()),
                         ec.message(),
                         reqParams.Endpoint);
                socket->Close();
                co_await WaitForTimer(timer, connectionRetry);
                continue;
            }

            boost::beast::http::response<boost::beast::http::dynamic_body> res;
            co_await socket->AsyncRead(res, ec);

            if (ec)
            {
                LogDebug("Error reading response ({}): {}. Endpoint: {}.",
                         std::to_string(ec.value()),
                         ec.message(),
                         reqParams.Endpoint);
                socket->Close();
                co_await WaitForTimer(timer, connectionRetry);
                continue;
            }

            std::time_t timerSleep = A_SECOND_IN_MILLIS;

            if (res.result() >= boost::beast::http::status::ok &&
                res.result() < boost::beast::http::status::multiple_choices)
            {
                if (onSuccess != nullptr)
                {
                    onSuccess(messagesCount, boost::beast::buffers_to_string(res.body().data()));
                }
            }
            else if (res.result() == boost::beast::http::status::unauthorized ||
                     res.result() == boost::beast::http::status::forbidden)
            {
                if (onUnauthorized != nullptr)
                {
                    onUnauthorized();
                }
                timerSleep = connectionRetry;
            }

            LogDebug("Request {}: Status {}", reqParams.Endpoint, res.result_int());
            LogTrace("{}", ResponseToString(reqParams.Endpoint, res));

            co_await WaitForTimer(timer, timerSleep);
        } while (loopRequestCondition != nullptr && loopRequestCondition());
    }

    boost::beast::http::response<boost::beast::http::dynamic_body>
    HttpClient::PerformHttpRequest(const HttpRequestParams& params)
    {
        return PerformHttpRequestInternal(params,
                                          [](std::unique_ptr<IHttpSocket>& socket,
                                             boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                             boost::system::error_code& ec) { socket->Read(res, ec); });
    }

    std::optional<std::string> HttpClient::AuthenticateWithUuidAndKey(const std::string& serverUrl,
                                                                      const std::string& userAgent,
                                                                      const std::string& uuid,
                                                                      const std::string& key,
                                                                      const std::string& verificationMode)
    {
        const std::string body = R"({"uuid":")" + uuid + R"(", "key":")" + key + "\"}";
        const auto reqParams = http_client::HttpRequestParams(boost::beast::http::verb::post,
                                                              serverUrl,
                                                              "/api/v1/authentication",
                                                              userAgent,
                                                              verificationMode,
                                                              "",
                                                              "",
                                                              body);

        const auto res = PerformHttpRequest(reqParams);

        if (res.result() < boost::beast::http::status::ok ||
            res.result() >= boost::beast::http::status::multiple_choices)
        {
            if (res.result() == boost::beast::http::status::unauthorized ||
                res.result() == boost::beast::http::status::forbidden)
            {
                std::string message {};

                try
                {
                    message = nlohmann::json::parse(boost::beast::buffers_to_string(res.body().data()))
                                  .at("message")
                                  .get_ref<const std::string&>();
                }
                catch (const std::exception& e)
                {
                    LogError("Error parsing message in response: {}.", e.what());
                }

                if (message == "Invalid key" || message == "Agent does not exist")
                {
                    throw std::runtime_error(message);
                }
            }
            LogWarn("Error: {}.", res.result_int());
            return std::nullopt;
        }

        try
        {
            return nlohmann::json::parse(boost::beast::buffers_to_string(res.body().data()))
                .at("token")
                .get_ref<const std::string&>();
        }
        catch (const std::exception& e)
        {
            LogError("Error parsing token in response: {}.", e.what());
        }

        return std::nullopt;
    }

    std::optional<std::string> HttpClient::AuthenticateWithUserPassword(const std::string& serverUrl,
                                                                        const std::string& userAgent,
                                                                        const std::string& user,
                                                                        const std::string& password,
                                                                        const std::string& verificationMode)
    {
        std::string basicAuth {};
        std::string userPass {user + ":" + password};

        basicAuth.resize(boost::beast::detail::base64::encoded_size(userPass.size()));

        boost::beast::detail::base64::encode(&basicAuth[0], userPass.c_str(), userPass.size());

        const auto reqParams = http_client::HttpRequestParams(boost::beast::http::verb::post,
                                                              serverUrl,
                                                              "/security/user/authenticate",
                                                              userAgent,
                                                              verificationMode,
                                                              "",
                                                              basicAuth);

        const auto res = PerformHttpRequest(reqParams);

        if (res.result() < boost::beast::http::status::ok ||
            res.result() >= boost::beast::http::status::multiple_choices)
        {
            LogWarn("Error: {}.", res.result_int());
            return std::nullopt;
        }

        try
        {
            return nlohmann::json::parse(boost::beast::buffers_to_string(res.body().data()))
                .at("data")
                .at("token")
                .get_ref<const std::string&>();
        }
        catch (const std::exception& e)
        {
            LogError("Error parsing token in response: {}.", e.what());
        }

        return std::nullopt;
    }

    boost::beast::http::response<boost::beast::http::dynamic_body> HttpClient::PerformHttpRequestInternal(
        const HttpRequestParams& params,
        const std::function<void(std::unique_ptr<IHttpSocket>&,
                                 boost::beast::http::response<boost::beast::http::dynamic_body>&,
                                 boost::system::error_code&)>& responseHandler)
    {
        boost::beast::http::response<boost::beast::http::dynamic_body> res;

        try
        {
            boost::asio::io_context io_context;
            auto resolver = m_resolverFactory->Create(io_context.get_executor());

            const auto results = resolver->Resolve(params.Host, params.Port);

            if (results.empty())
            {
                throw std::runtime_error("Failed to resolve host.");
            }

            auto socket = m_socketFactory->Create(io_context.get_executor(), params.Use_Https);

            if (!socket)
            {
                throw std::runtime_error("Failed to create socket.");
            }

            if (params.Use_Https)
            {
                socket->SetVerificationMode(params.Host, params.Verification_Mode);
            }

            boost::system::error_code ec;

            socket->Connect(results, ec);

            if (ec)
            {
                throw std::runtime_error("Error connecting to host: " + ec.message());
            }

            const auto req = CreateHttpRequest(params);

            socket->Write(req, ec);

            if (ec)
            {
                throw std::runtime_error("Error writing request: " + ec.message());
            }

            responseHandler(socket, res, ec);

            if (ec)
            {
                throw std::runtime_error("Error handling response: " + ec.message());
            }

            LogDebug("Request {}: Status {}", params.Endpoint, res.result_int());
            LogTrace("{}", ResponseToString(params.Endpoint, res));
        }
        catch (std::exception const& e)
        {
            LogError("Error: {}", e.what());

            res.result(boost::beast::http::status::internal_server_error);
            boost::beast::ostream(res.body()) << "Internal server error: " << e.what();
            res.prepare_payload();
        }

        return res;
    }
} // namespace http_client
