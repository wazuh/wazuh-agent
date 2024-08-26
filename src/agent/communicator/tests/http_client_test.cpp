#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <http_client.hpp>

#include <ihttp_resolver.hpp>
#include <ihttp_resolver_factory.hpp>
#include <ihttp_socket.hpp>
#include <ihttp_socket_factory.hpp>

#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <string>

class MockHttpResolver : public http_client::IHttpResolver
{
public:
    MOCK_METHOD(boost::asio::ip::tcp::resolver::results_type,
                Resolve,
                (const std::string& host, const std::string& port),
                (override));

    MOCK_METHOD(boost::asio::awaitable<boost::asio::ip::tcp::resolver::results_type>,
                AsyncResolve,
                (const std::string& host, const std::string& port),
                (override));
};

class MockHttpSocket : public http_client::IHttpSocket
{
public:
    MOCK_METHOD(void, Connect, (const boost::asio::ip::tcp::resolver::results_type& endpoints), (override));

    MOCK_METHOD(boost::asio::awaitable<void>,
                AsyncConnect,
                (const boost::asio::ip::tcp::resolver::results_type& endpoints, boost::system::error_code& code),
                (override));

    MOCK_METHOD(void, Write, (const boost::beast::http::request<boost::beast::http::string_body>& req), (override));

    MOCK_METHOD(boost::asio::awaitable<void>,
                AsyncWrite,
                (const boost::beast::http::request<boost::beast::http::string_body>& req, boost::beast::error_code& ec),
                (override));

    MOCK_METHOD(void, Read, (boost::beast::http::response<boost::beast::http::dynamic_body> & res), (override));

    MOCK_METHOD(boost::asio::awaitable<void>,
                AsyncRead,
                (boost::beast::http::response<boost::beast::http::dynamic_body> & res, boost::beast::error_code& ec),
                (override));

    MOCK_METHOD(void, Close, (), (override));
};

class MockHttpResolverFactory : public http_client::IHttpResolverFactory
{
public:
    MOCK_METHOD(std::unique_ptr<http_client::IHttpResolver>,
                Create,
                (const boost::asio::any_io_executor& executor),
                (override));
};

class MockHttpSocketFactory : public http_client::IHttpSocketFactory
{
public:
    MOCK_METHOD(std::unique_ptr<http_client::IHttpSocket>,
                Create,
                (const boost::asio::any_io_executor& executor),
                (override));
};

using namespace testing;

class HttpClientTest : public Test
{
protected:
    void SetUp() override
    {
        mockResolverFactory = std::make_shared<MockHttpResolverFactory>();
        mockSocketFactory = std::make_shared<MockHttpSocketFactory>();
        mockResolver = std::make_unique<MockHttpResolver>();
        mockSocket = std::make_unique<MockHttpSocket>();
        client = std::make_unique<http_client::HttpClient>(mockResolverFactory, mockSocketFactory);
    }

    std::shared_ptr<MockHttpResolverFactory> mockResolverFactory;
    std::shared_ptr<MockHttpSocketFactory> mockSocketFactory;
    std::unique_ptr<MockHttpResolver> mockResolver;
    std::unique_ptr<MockHttpSocket> mockSocket;
    std::unique_ptr<http_client::HttpClient> client;
};

TEST(CreateHttpRequestTest, BasicGetRequest)
{
    auto httpClient = http_client::HttpClient();
    const auto reqParams = http_client::HttpRequestParams(boost::beast::http::verb::get, "localhost", "8080", "/test");
    const auto req = httpClient.CreateHttpRequest(reqParams);

    EXPECT_EQ(req.method(), boost::beast::http::verb::get);
    EXPECT_EQ(req.target(), "/test");
    EXPECT_EQ(req.version(), 11);
    EXPECT_EQ(req[boost::beast::http::field::host], "localhost");
    EXPECT_EQ(req[boost::beast::http::field::user_agent], BOOST_BEAST_VERSION_STRING);
    EXPECT_EQ(req[boost::beast::http::field::accept], "application/json");
}

TEST(CreateHttpRequestTest, PostRequestWithBody)
{
    auto httpClient = http_client::HttpClient();
    const std::string body = R"({"key": "value"})";
    const auto reqParams =
        http_client::HttpRequestParams(boost::beast::http::verb::post, "localhost", "8080", "/submit", "", "", body);
    const auto req = httpClient.CreateHttpRequest(reqParams);

    EXPECT_EQ(req.method(), boost::beast::http::verb::post);
    EXPECT_EQ(req.target(), "/submit");
    EXPECT_EQ(req.version(), 11);
    EXPECT_EQ(req[boost::beast::http::field::host], "localhost");
    EXPECT_EQ(req[boost::beast::http::field::user_agent], BOOST_BEAST_VERSION_STRING);
    EXPECT_EQ(req[boost::beast::http::field::accept], "application/json");
    EXPECT_EQ(req[boost::beast::http::field::content_type], "application/json");
    EXPECT_EQ(req.body(), body);
}

TEST(CreateHttpRequestTest, AuthorizationBearerToken)
{
    auto httpClient = http_client::HttpClient();
    const std::string token = "dummy_token";
    const auto reqParams =
        http_client::HttpRequestParams(boost::beast::http::verb::get, "localhost", "8080", "/secure", token);
    const auto req = httpClient.CreateHttpRequest(reqParams);

    EXPECT_EQ(req[boost::beast::http::field::authorization], "Bearer dummy_token");
}

TEST(CreateHttpRequestTest, AuthorizationBasic)
{
    auto httpClient = http_client::HttpClient();
    const std::string user_pass = "username:password";
    const auto reqParams =
        http_client::HttpRequestParams(boost::beast::http::verb::get, "localhost", "8080", "/secure", "", user_pass);
    const auto req = httpClient.CreateHttpRequest(reqParams);

    EXPECT_EQ(req[boost::beast::http::field::authorization], "Basic username:password");
}

TEST_F(HttpClientTest, PerformHttpRequest_Success)
{
    EXPECT_CALL(*mockResolverFactory, Create(_))
        .WillOnce(Invoke(
            [&](const auto& executor) -> std::unique_ptr<http_client::IHttpResolver>
            {
                EXPECT_TRUE(executor);
                return std::move(mockResolver);
            }));

    EXPECT_CALL(*mockSocketFactory, Create(_))
        .WillOnce(Invoke(
            [&](const auto& executor) -> std::unique_ptr<http_client::IHttpSocket>
            {
                EXPECT_TRUE(executor);
                return std::move(mockSocket);
            }));

    EXPECT_CALL(*mockResolver, Resolve(_, _)).WillOnce(Return(boost::asio::ip::tcp::resolver::results_type {}));
    EXPECT_CALL(*mockSocket, Connect(_)).Times(1);
    EXPECT_CALL(*mockSocket, Write(_)).Times(1);
    EXPECT_CALL(*mockSocket, Read(_)).WillOnce([](auto& res) { res.result(boost::beast::http::status::ok); });

    const http_client::HttpRequestParams params(boost::beast::http::verb::get, "localhost", "80", "/");
    const auto response = client->PerformHttpRequest(params);

    EXPECT_EQ(response.result(), boost::beast::http::status::ok);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
