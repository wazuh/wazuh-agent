#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <http_client.hpp>

#include "mocks/mock_http_resolver.hpp"
#include "mocks/mock_http_resolver_factory.hpp"
#include "mocks/mock_http_socket.hpp"
#include "mocks/mock_http_socket_factory.hpp"

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <functional>
#include <memory>
#include <string>
#include <utility>

// NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines,cppcoreguidelines-avoid-reference-coroutine-parameters)

using namespace testing;

class HttpClientTest : public TestWithParam<boost::beast::http::status>
{
protected:
    HttpClientTest()
    {
        const auto port = 80;
        dummyResults = boost::asio::ip::tcp::resolver::results_type::create(
            boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), port),
            "127.0.0.1",
            "80");
    }

    void SetUp() override
    {
        mockResolverFactory = std::make_shared<MockHttpResolverFactory>();
        mockSocketFactory = std::make_shared<MockHttpSocketFactory>();
        mockResolver = std::make_unique<MockHttpResolver>();
        mockSocket = std::make_unique<MockHttpSocket>();
        client = std::make_unique<http_client::HttpClient>(mockResolverFactory, mockSocketFactory);
    }

    void SetupMockResolverFactory()
    {
        EXPECT_CALL(*mockResolverFactory, Create(_))
            .WillOnce(Invoke(
                [&](const auto& executor) -> std::unique_ptr<http_client::IHttpResolver>
                {
                    EXPECT_TRUE(executor);
                    return std::move(mockResolver);
                }));
    }

    void SetupMockSocketFactory()
    {
        EXPECT_CALL(*mockSocketFactory, Create(_, _))
            .WillOnce(Invoke(
                [&](const auto& executor,
                    [[maybe_unused]] const bool useHttps) -> std::unique_ptr<http_client::IHttpSocket>
                {
                    EXPECT_TRUE(executor);
                    return std::move(mockSocket);
                }));
    }

    void SetupMockResolverExpectations()
    {
        EXPECT_CALL(*mockResolver, AsyncResolve(_, _))
            .WillOnce(Invoke([this](const std::string&, const std::string&)
                                 -> boost::asio::awaitable<boost::asio::ip::tcp::resolver::results_type>
                             { co_return dummyResults; }));
    }

    void SetupMockSocketConnectExpectations(boost::system::error_code connectEc = {})
    {
        EXPECT_CALL(*mockSocket, AsyncConnect(_, _))
            .WillOnce(Invoke(
                [connectEc](const boost::asio::ip::tcp::resolver::results_type&,
                            boost::system::error_code& ec) -> boost::asio::awaitable<void>
                {
                    ec = connectEc;
                    co_return;
                }));
    }

    void SetupMockSocketWriteExpectations(boost::system::error_code writeEc = {})
    {
        EXPECT_CALL(*mockSocket, AsyncWrite(_, _))
            .WillOnce(Invoke(
                [writeEc](const boost::beast::http::request<boost::beast::http::string_body>&,
                          boost::system::error_code& ec) -> boost::asio::awaitable<void>
                {
                    ec = writeEc;
                    co_return;
                }));
    }

    void SetupMockSocketReadExpectations(boost::beast::http::status status, boost::system::error_code readEc = {})
    {
        EXPECT_CALL(*mockSocket, AsyncRead(_, _))
            .WillOnce(Invoke(
                [status, readEc](auto& res, boost::system::error_code& ec) -> boost::asio::awaitable<void>
                {
                    res.result(status);
                    ec = readEc;
                    co_return;
                }));
    }

    std::shared_ptr<MockHttpResolverFactory> mockResolverFactory;
    std::shared_ptr<MockHttpSocketFactory> mockSocketFactory;
    std::unique_ptr<MockHttpResolver> mockResolver;
    std::unique_ptr<MockHttpSocket> mockSocket;
    std::unique_ptr<http_client::HttpClient> client;

    boost::asio::ip::tcp::resolver::results_type dummyResults;
};

TEST(CreateHttpRequestTest, BasicGetRequest)
{
    auto httpClient = http_client::HttpClient();
    const auto reqParams = http_client::HttpRequestParams(
        boost::beast::http::verb::get, "https://localhost", "/test", "Wazuh 5.0.0", "full");
    const auto req = httpClient.CreateHttpRequest(reqParams);

    EXPECT_EQ(req.method(), boost::beast::http::verb::get);
    EXPECT_EQ(req.target(), "/test");
    EXPECT_EQ(req.version(), 11);
    EXPECT_EQ(req[boost::beast::http::field::host], "localhost");
    EXPECT_EQ(req[boost::beast::http::field::user_agent], "Wazuh 5.0.0");
    EXPECT_EQ(req[boost::beast::http::field::accept], "application/json");
}

TEST(CreateHttpRequestTest, PostRequestWithBody)
{
    auto httpClient = http_client::HttpClient();
    const std::string body = R"({"key": "value"})";
    const auto reqParams = http_client::HttpRequestParams(
        boost::beast::http::verb::post, "https://localhost:8080", "/submit", "Wazuh 5.0.0", "full", "", "", body);
    const auto req = httpClient.CreateHttpRequest(reqParams);

    EXPECT_EQ(req.method(), boost::beast::http::verb::post);
    EXPECT_EQ(req.target(), "/submit");
    EXPECT_EQ(req.version(), 11);
    EXPECT_EQ(req[boost::beast::http::field::host], "localhost");
    EXPECT_EQ(req[boost::beast::http::field::user_agent], "Wazuh 5.0.0");
    EXPECT_EQ(req[boost::beast::http::field::accept], "application/json");
    EXPECT_EQ(req[boost::beast::http::field::content_type], "application/json");
    EXPECT_EQ(req.body(), body);
}

TEST(CreateHttpRequestTest, AuthorizationBearerToken)
{
    auto httpClient = http_client::HttpClient();
    const std::string token = "dummy_token";
    const auto reqParams = http_client::HttpRequestParams(
        boost::beast::http::verb::get, "localhost", "/secure", "Wazuh 5.0.0", "full", token);
    const auto req = httpClient.CreateHttpRequest(reqParams);

    EXPECT_EQ(req[boost::beast::http::field::user_agent], "Wazuh 5.0.0");
    EXPECT_EQ(req[boost::beast::http::field::authorization], "Bearer dummy_token");
}

TEST(CreateHttpRequestTest, AuthorizationBasic)
{
    auto httpClient = http_client::HttpClient();
    const std::string user_pass = "username:password";
    const auto reqParams = http_client::HttpRequestParams(
        boost::beast::http::verb::get, "https://localhost:8080", "/secure", "Wazuh 5.0.0", "full", "", user_pass);
    const auto req = httpClient.CreateHttpRequest(reqParams);

    EXPECT_EQ(req[boost::beast::http::field::user_agent], "Wazuh 5.0.0");
    EXPECT_EQ(req[boost::beast::http::field::authorization], "Basic username:password");
}

TEST_F(HttpClientTest, PerformHttpRequest_Success_verification_full)
{
    SetupMockResolverFactory();
    SetupMockSocketFactory();

    EXPECT_CALL(*mockResolver, Resolve(_, _)).WillOnce(Return(dummyResults));
    EXPECT_CALL(*mockSocket, Connect(_, _)).Times(1);
    EXPECT_CALL(*mockSocket, SetVerificationMode("localhost", "full")).Times(1);
    EXPECT_CALL(*mockSocket, Write(_, _)).Times(1);
    EXPECT_CALL(*mockSocket, Read(_, _)).WillOnce([](auto& res, auto&) { res.result(boost::beast::http::status::ok); });

    const http_client::HttpRequestParams params(
        boost::beast::http::verb::get, "https://localhost:80", "/", "Wazuh 5.0.0", "full");
    const auto response = client->PerformHttpRequest(params);

    EXPECT_EQ(response.result(), boost::beast::http::status::ok);
}

TEST_F(HttpClientTest, PerformHttpRequest_Success_verification_certificate)
{
    SetupMockResolverFactory();
    SetupMockSocketFactory();

    EXPECT_CALL(*mockResolver, Resolve(_, _)).WillOnce(Return(dummyResults));
    EXPECT_CALL(*mockSocket, Connect(_, _)).Times(1);
    EXPECT_CALL(*mockSocket, SetVerificationMode("localhost", "certificate")).Times(1);
    EXPECT_CALL(*mockSocket, Write(_, _)).Times(1);
    EXPECT_CALL(*mockSocket, Read(_, _)).WillOnce([](auto& res, auto&) { res.result(boost::beast::http::status::ok); });

    const http_client::HttpRequestParams params(
        boost::beast::http::verb::get, "https://localhost:80", "/", "Wazuh 5.0.0", "certificate");
    const auto response = client->PerformHttpRequest(params);

    EXPECT_EQ(response.result(), boost::beast::http::status::ok);
}

TEST_F(HttpClientTest, PerformHttpRequest_Success_verification_none)
{
    SetupMockResolverFactory();
    SetupMockSocketFactory();

    EXPECT_CALL(*mockResolver, Resolve(_, _)).WillOnce(Return(dummyResults));
    EXPECT_CALL(*mockSocket, Connect(_, _)).Times(1);
    EXPECT_CALL(*mockSocket, SetVerificationMode("localhost", "none")).Times(1);
    EXPECT_CALL(*mockSocket, Write(_, _)).Times(1);
    EXPECT_CALL(*mockSocket, Read(_, _)).WillOnce([](auto& res, auto&) { res.result(boost::beast::http::status::ok); });

    const http_client::HttpRequestParams params(
        boost::beast::http::verb::get, "https://localhost:80", "/", "Wazuh 5.0.0", "none");
    const auto response = client->PerformHttpRequest(params);

    EXPECT_EQ(response.result(), boost::beast::http::status::ok);
}

TEST_F(HttpClientTest, PerformHttpRequest_ExceptionThrown)
{
    SetupMockResolverFactory();

    EXPECT_CALL(*mockResolver, Resolve(_, _)).WillOnce(Throw(std::runtime_error("Simulated resolution failure")));

    const http_client::HttpRequestParams params(
        boost::beast::http::verb::get, "https://localhost:80", "/", "Wazuh 5.0.0", "full");
    const auto response = client->PerformHttpRequest(params);

    EXPECT_EQ(response.result(), boost::beast::http::status::internal_server_error);
    EXPECT_TRUE(boost::beast::buffers_to_string(response.body().data()).find("Simulated resolution failure") !=
                std::string::npos);
}

TEST_P(HttpClientTest, Co_PerformHttpRequest_Success)
{
    SetupMockResolverFactory();
    SetupMockSocketFactory();
    SetupMockResolverExpectations();
    SetupMockSocketConnectExpectations();
    EXPECT_CALL(*mockSocket, SetVerificationMode("localhost", "full")).Times(1);
    SetupMockSocketWriteExpectations();
    SetupMockSocketReadExpectations(GetParam());

    auto getMessagesCalled = false;
    auto getMessages = [&getMessagesCalled](const size_t) -> boost::asio::awaitable<std::tuple<int, std::string>>
    {
        getMessagesCalled = true;
        co_return std::tuple<int, std::string>(1, "test message");
    };

    auto onSuccessCalled = false;
    std::function<void(const int, const std::string&)> onSuccess = [&onSuccessCalled](const int, const std::string&)
    {
        onSuccessCalled = true;
    };

    auto unauthorizedCalled = false;
    std::function<void()> onUnauthorized = [&unauthorizedCalled]()
    {
        unauthorizedCalled = true;
    };

    auto loopCondition = true;
    std::function<bool()> loopRequestCondition = [&loopCondition]()
    {
        return std::exchange(loopCondition, false);
    };

    const auto reqParams = http_client::HttpRequestParams(
        boost::beast::http::verb::get, "https://localhost:8080", "/", "Wazuh 5.0.0", "full");

    auto task = client->Co_PerformHttpRequest(std::make_shared<std::string>("token"),
                                              reqParams,
                                              getMessages,
                                              onUnauthorized,
                                              5, // NOLINT
                                              1, // NOLINT
                                              onSuccess,
                                              loopRequestCondition);

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(ioContext, std::move(task), boost::asio::detached);
    ioContext.run();

    EXPECT_TRUE(getMessagesCalled);
    EXPECT_FALSE(unauthorizedCalled);
    EXPECT_TRUE(onSuccessCalled);
}

INSTANTIATE_TEST_SUITE_P(HttpClientTest,
                         HttpClientTest,
                         testing::ValuesIn({boost::beast::http::status::ok,
                                            boost::beast::http::status::created,
                                            boost::beast::http::status::accepted,
                                            boost::beast::http::status::no_content}));

TEST_F(HttpClientTest, Co_PerformHttpRequest_CallbacksNotCalledIfCannotConnect)
{
    SetupMockResolverFactory();
    SetupMockSocketFactory();
    SetupMockResolverExpectations();
    SetupMockSocketConnectExpectations(boost::system::errc::make_error_code(boost::system::errc::bad_address));
    EXPECT_CALL(*mockSocket, SetVerificationMode("localhost", "full")).Times(1);

    auto getMessagesCalled = false;
    auto getMessages = [&getMessagesCalled](const size_t) -> boost::asio::awaitable<std::tuple<int, std::string>>
    {
        getMessagesCalled = true;
        co_return std::tuple<int, std::string>(1, "test message");
    };

    auto onSuccessCalled = false;
    std::function<void(const int, const std::string&)> onSuccess = [&onSuccessCalled](const int, const std::string&)
    {
        onSuccessCalled = true;
    };

    auto unauthorizedCalled = false;
    std::function<void()> onUnauthorized = [&unauthorizedCalled]()
    {
        unauthorizedCalled = true;
    };

    const auto reqParams = http_client::HttpRequestParams(
        boost::beast::http::verb::get, "https://localhost:8080", "/", "Wazuh 5.0.0", "full");
    auto task = client->Co_PerformHttpRequest(std::make_shared<std::string>("token"),
                                              reqParams,
                                              getMessages,
                                              onUnauthorized,
                                              5, // NOLINT
                                              1, // NOLINT
                                              onSuccess,
                                              nullptr);

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(ioContext, std::move(task), boost::asio::detached);
    ioContext.run();

    EXPECT_FALSE(getMessagesCalled);
    EXPECT_FALSE(unauthorizedCalled);
    EXPECT_FALSE(onSuccessCalled);
}

TEST_F(HttpClientTest, Co_PerformHttpRequest_OnSuccessNotCalledIfAsyncWriteFails)
{
    SetupMockResolverFactory();
    SetupMockSocketFactory();
    SetupMockResolverExpectations();
    SetupMockSocketConnectExpectations(boost::system::errc::make_error_code(boost::system::errc::success));
    EXPECT_CALL(*mockSocket, SetVerificationMode("localhost", "full")).Times(1);
    SetupMockSocketWriteExpectations(boost::system::errc::make_error_code(boost::system::errc::bad_address));

    auto getMessagesCalled = false;
    auto getMessages = [&getMessagesCalled](const size_t) -> boost::asio::awaitable<std::tuple<int, std::string>>
    {
        getMessagesCalled = true;
        co_return std::tuple<int, std::string>(1, "test message");
    };

    auto onSuccessCalled = false;
    std::function<void(const int, const std::string&)> onSuccess = [&onSuccessCalled](const int, const std::string&)
    {
        onSuccessCalled = true;
    };

    auto unauthorizedCalled = false;
    std::function<void()> onUnauthorized = [&unauthorizedCalled]()
    {
        unauthorizedCalled = true;
    };

    auto loopCondition = true;
    std::function<bool()> loopRequestCondition = [&loopCondition]()
    {
        return std::exchange(loopCondition, false);
    };

    const auto reqParams = http_client::HttpRequestParams(
        boost::beast::http::verb::get, "https://localhost:8080", "/", "Wazuh 5.0.0", "full");
    auto task = client->Co_PerformHttpRequest(std::make_shared<std::string>("token"),
                                              reqParams,
                                              getMessages,
                                              onUnauthorized,
                                              5, // NOLINT
                                              1, // NOLINT
                                              onSuccess,
                                              loopRequestCondition);

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(ioContext, std::move(task), boost::asio::detached);
    ioContext.run();

    EXPECT_TRUE(getMessagesCalled);
    EXPECT_FALSE(unauthorizedCalled);
    EXPECT_FALSE(onSuccessCalled);
}

TEST_F(HttpClientTest, Co_PerformHttpRequest_OnSuccessNotCalledIfAsyncReadFails)
{
    SetupMockResolverFactory();
    SetupMockSocketFactory();
    SetupMockResolverExpectations();
    SetupMockSocketConnectExpectations(boost::system::errc::make_error_code(boost::system::errc::success));
    EXPECT_CALL(*mockSocket, SetVerificationMode("localhost", "full")).Times(1);
    SetupMockSocketWriteExpectations();
    SetupMockSocketReadExpectations(boost::beast::http::status::not_found,
                                    boost::system::errc::make_error_code(boost::system::errc::bad_address));

    auto getMessagesCalled = false;
    auto getMessages = [&getMessagesCalled](const size_t) -> boost::asio::awaitable<std::tuple<int, std::string>>
    {
        getMessagesCalled = true;
        co_return std::tuple<int, std::string>(1, "test message");
    };

    auto onSuccessCalled = false;
    std::function<void(const int, const std::string&)> onSuccess = [&onSuccessCalled](const int, const std::string&)
    {
        onSuccessCalled = true;
    };

    auto unauthorizedCalled = false;
    std::function<void()> onUnauthorized = [&unauthorizedCalled]()
    {
        unauthorizedCalled = true;
    };

    auto loopCondition = true;
    std::function<bool()> loopRequestCondition = [&loopCondition]()
    {
        return std::exchange(loopCondition, false);
    };

    const auto reqParams = http_client::HttpRequestParams(
        boost::beast::http::verb::get, "https://localhost:8080", "/", "Wazuh 5.0.0", "full");
    auto task = client->Co_PerformHttpRequest(std::make_shared<std::string>("token"),
                                              reqParams,
                                              getMessages,
                                              onUnauthorized,
                                              5, // NOLINT
                                              1, // NOLINT
                                              onSuccess,
                                              loopRequestCondition);

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(ioContext, std::move(task), boost::asio::detached);
    ioContext.run();

    EXPECT_TRUE(getMessagesCalled);
    EXPECT_FALSE(unauthorizedCalled);
    EXPECT_FALSE(onSuccessCalled);
}

TEST_F(HttpClientTest, Co_PerformHttpRequest_UnauthorizedCalledWhenAuthorizationFails)
{
    SetupMockResolverFactory();
    SetupMockSocketFactory();
    SetupMockResolverExpectations();
    SetupMockSocketConnectExpectations(boost::system::errc::make_error_code(boost::system::errc::success));
    EXPECT_CALL(*mockSocket, SetVerificationMode("localhost", "full")).Times(1);
    SetupMockSocketWriteExpectations();
    SetupMockSocketReadExpectations(boost::beast::http::status::unauthorized);

    auto getMessagesCalled = false;
    auto getMessages = [&getMessagesCalled](const size_t) -> boost::asio::awaitable<std::tuple<int, std::string>>
    {
        getMessagesCalled = true;
        co_return std::tuple<int, std::string>(1, "test message");
    };

    auto onSuccessCalled = false;
    std::function<void(const int, const std::string&)> onSuccess = [&onSuccessCalled](const int, const std::string&)
    {
        onSuccessCalled = true;
    };

    auto unauthorizedCalled = false;
    std::function<void()> onUnauthorized = [&unauthorizedCalled]()
    {
        unauthorizedCalled = true;
    };

    auto loopCondition = true;
    std::function<bool()> loopRequestCondition = [&loopCondition]()
    {
        return std::exchange(loopCondition, false);
    };

    const auto reqParams = http_client::HttpRequestParams(
        boost::beast::http::verb::get, "https://localhost:8080", "/", "Wazuh 5.0.0", "full");
    auto task = client->Co_PerformHttpRequest(std::make_shared<std::string>("token"),
                                              reqParams,
                                              getMessages,
                                              onUnauthorized,
                                              5, // NOLINT
                                              1, // NOLINT
                                              onSuccess,
                                              loopRequestCondition);

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(ioContext, std::move(task), boost::asio::detached);
    ioContext.run();

    EXPECT_TRUE(getMessagesCalled);
    EXPECT_TRUE(unauthorizedCalled);
    EXPECT_FALSE(onSuccessCalled);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines,cppcoreguidelines-avoid-reference-coroutine-parameters)
