#include <gmock/gmock.h>

#include <ihttp_client.hpp>

using intStringTuple = std::tuple<int, std::string>;

class MockHttpClient : public http_client::IHttpClient
{
public:
    MOCK_METHOD(boost::beast::http::request<boost::beast::http::string_body>,
                CreateHttpRequest,
                (const http_client::HttpRequestParams& params),
                (override));

    MOCK_METHOD(boost::asio::awaitable<void>,
                Co_PerformHttpRequest,
                (std::shared_ptr<std::string> token,
                 http_client::HttpRequestParams params,
                 std::function<boost::asio::awaitable<intStringTuple>(const size_t)> bodyGetter,
                 std::function<void()> onUnauthorized,
                 std::time_t connectionRetry,
                 size_t batchSize,
                 std::function<void(const int, const std::string&)> onSuccess,
                 std::function<bool()> loopRequestCondition),
                (override));

    MOCK_METHOD(boost::beast::http::response<boost::beast::http::dynamic_body>,
                PerformHttpRequest,
                (const http_client::HttpRequestParams& params),
                (override));
};
