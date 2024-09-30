#include <gmock/gmock.h>

#include <ihttp_socket.hpp>
#include <ihttp_socket_factory.hpp>

class MockHttpSocketFactory : public http_client::IHttpSocketFactory
{
public:
    MOCK_METHOD(std::unique_ptr<http_client::IHttpSocket>,
                Create,
                (const boost::asio::any_io_executor& executor, const bool use_https),
                (override));
};
