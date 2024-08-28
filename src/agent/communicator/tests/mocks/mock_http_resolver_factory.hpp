#include <gmock/gmock.h>

#include <ihttp_resolver.hpp>
#include <ihttp_resolver_factory.hpp>

class MockHttpResolverFactory : public http_client::IHttpResolverFactory
{
public:
    MOCK_METHOD(std::unique_ptr<http_client::IHttpResolver>,
                Create,
                (const boost::asio::any_io_executor& executor),
                (override));
};
