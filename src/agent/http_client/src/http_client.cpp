#include <http_client.hpp>

#include "http_resolver_factory.hpp"
#include "http_socket_factory.hpp"
#include "ihttp_resolver_factory.hpp"
#include "ihttp_socket_factory.hpp"

#include <boost/asio.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/http.hpp>

#include <logger.hpp>

#include <string>

namespace
{
    boost::beast::http::verb GetRequestMethod(http_client::MethodType method)
    {
        switch (method)
        {
            case http_client::MethodType::GET: return boost::beast::http::verb::get;
            case http_client::MethodType::POST: return boost::beast::http::verb::post;
            case http_client::MethodType::PUT: return boost::beast::http::verb::put;
            case http_client::MethodType::DELETE_: return boost::beast::http::verb::delete_;
            default: return boost::beast::http::verb::get;
        }
    }

    boost::beast::http::request<boost::beast::http::string_body>
    CreateHttpRequest(const http_client::HttpRequestParams& params)
    {
        static constexpr int HttpVersion1_1 = 11;

        boost::beast::http::request<boost::beast::http::string_body> req {
            GetRequestMethod(params.Method), params.Endpoint, HttpVersion1_1};
        req.set(boost::beast::http::field::host, params.Host);
        req.set(boost::beast::http::field::user_agent, params.User_agent);
        req.set(boost::beast::http::field::accept, "application/json");

        if (!params.Token.empty())
        {
            req.set(boost::beast::http::field::authorization, "Bearer " + params.Token);
        }

        if (!params.User_pass.empty())
        {
            std::string basicAuth {};

            basicAuth.resize(boost::beast::detail::base64::encoded_size(params.User_pass.size()));

            boost::beast::detail::base64::encode(&basicAuth[0], params.User_pass.c_str(), params.User_pass.size());

            req.set(boost::beast::http::field::authorization, "Basic " + basicAuth);
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

    boost::asio::awaitable<std::tuple<int, std::string>>
    HttpClient::Co_PerformHttpRequest(const HttpRequestParams params)
    {
        boost::beast::http::response<boost::beast::http::dynamic_body> res;

        try
        {
            auto executor = co_await boost::asio::this_coro::executor;
            auto resolver = m_resolverFactory->Create(executor);

            const auto results = co_await resolver->AsyncResolve(params.Host, params.Port);

            if (results.empty())
            {
                throw std::runtime_error("Failed to resolve host.");
            }

            auto socket = m_socketFactory->Create(executor, params.Use_Https);

            if (!socket)
            {
                throw std::runtime_error("Failed to create socket.");
            }

            if (params.Use_Https)
            {
                socket->SetVerificationMode(params.Host, params.Verification_Mode);
            }

            boost::system::error_code ec;

            co_await socket->AsyncConnect(results, ec);

            if (ec)
            {
                throw std::runtime_error("Error connecting to host: " + ec.message());
            }

            const auto req = CreateHttpRequest(params);

            co_await socket->AsyncWrite(req, ec);

            if (ec)
            {
                throw std::runtime_error("Error writing request: " + ec.message());
            }

            if (params.RequestTimeout)
            {
                co_await socket->AsyncRead(res, ec, params.RequestTimeout);
            }
            else
            {
                co_await socket->AsyncRead(res, ec);
            }

            if (ec)
            {
                throw std::runtime_error("Error handling response: " + ec.message());
            }

            LogDebug("Request {}: Status {}", params.Endpoint, res.result_int());
            LogTrace("{}", ResponseToString(params.Endpoint, res));
        }
        catch (std::exception const& e)
        {
            LogError("Error: {}. Endpoint: {}.", e.what(), params.Endpoint);

            res.result(boost::beast::http::status::internal_server_error);
            boost::beast::ostream(res.body()) << "Internal server error: " << e.what();
            res.prepare_payload();
        }

        co_return std::tuple<int, std::string> {res.result_int(), boost::beast::buffers_to_string(res.body().data())};
    }

    std::tuple<int, std::string> HttpClient::PerformHttpRequest(const HttpRequestParams& params)
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
            io_context.run();

            if (ec)
            {
                throw std::runtime_error("Error connecting to host: " + ec.message());
            }

            const auto req = CreateHttpRequest(params);

            socket->Write(req, ec);
            io_context.run();

            if (ec)
            {
                throw std::runtime_error("Error writing request: " + ec.message());
            }

            socket->Read(res, ec);
            io_context.run();

            if (ec)
            {
                throw std::runtime_error("Error handling response: " + ec.message());
            }

            LogDebug("Request {}: Status {}", params.Endpoint, res.result_int());
            LogTrace("{}", ResponseToString(params.Endpoint, res));
        }
        catch (std::exception const& e)
        {
            LogError("Error: {}. Endpoint: {}.", e.what(), params.Endpoint);

            res.result(boost::beast::http::status::internal_server_error);
            boost::beast::ostream(res.body()) << "Internal server error: " << e.what();
            res.prepare_payload();
        }

        return std::tuple<int, std::string> {res.result_int(), boost::beast::buffers_to_string(res.body().data())};
    }
} // namespace http_client
