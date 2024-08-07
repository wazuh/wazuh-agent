#include <gtest/gtest.h>

#include <http_client.hpp>

#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <string>

TEST(CreateHttpRequestTest, BasicGetRequest)
{
    const auto req = http_client::CreateHttpRequest(boost::beast::http::verb::get, "/test", "localhost", "", "", "");

    EXPECT_EQ(req.method(), boost::beast::http::verb::get);
    EXPECT_EQ(req.target(), "/test");
    EXPECT_EQ(req.version(), 11);
    EXPECT_EQ(req[boost::beast::http::field::host], "localhost");
    EXPECT_EQ(req[boost::beast::http::field::user_agent], BOOST_BEAST_VERSION_STRING);
    EXPECT_EQ(req[boost::beast::http::field::accept], "application/json");
}

TEST(CreateHttpRequestTest, PostRequestWithBody)
{
    const std::string body = R"({"key": "value"})";
    const auto req =
        http_client::CreateHttpRequest(boost::beast::http::verb::post, "/submit", "localhost", "", body, "");

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
    const std::string token = "dummy_token";
    const auto req =
        http_client::CreateHttpRequest(boost::beast::http::verb::get, "/secure", "localhost", token, "", "");

    EXPECT_EQ(req[boost::beast::http::field::authorization], "Bearer dummy_token");
}

TEST(CreateHttpRequestTest, AuthorizationBasic)
{
    const std::string user_pass = "username:password";
    const auto req =
        http_client::CreateHttpRequest(boost::beast::http::verb::get, "/secure", "localhost", "", "", user_pass);

    EXPECT_EQ(req[boost::beast::http::field::authorization], "Basic username:password");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
