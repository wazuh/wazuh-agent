#include <gmock/gmock.h>

#include <ihttp_resolver.hpp>

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
