#include <gmock/gmock.h>

#include <ihttp_client.hpp>

using intStringTuple = std::tuple<int, std::string>;

class MockHttpClient : public http_client::IHttpClient
{
public:
    MOCK_METHOD((boost::asio::awaitable<std::tuple<int, std::string>>),
                Co_PerformHttpRequest,
                (const http_client::HttpRequestParams params),
                (override));

    MOCK_METHOD((std::tuple<int, std::string>),
                PerformHttpRequest,
                (const http_client::HttpRequestParams& params),
                (override));
};
