#include <gmock/gmock.h>

#include <ihttp_socket.hpp>

class MockHttpHelper : public http_client::ISocketHelper
{
public:
    // MOCK_METHOD(void, SetVerificationMode, (const std::string&, const std::string&), (override));
    MOCK_METHOD(void, expires_after, (std::chrono::seconds), (override));
    MOCK_METHOD(void,
                connect,
                (const boost::asio::ip::tcp::resolver::results_type&, boost::system::error_code&),
                (override));
    MOCK_METHOD(boost::asio::awaitable<void>,
                async_connect,
                (const boost::asio::ip::tcp::resolver::results_type&, boost::system::error_code&),
                (override));
    MOCK_METHOD(void,
                write,
                (const boost::beast::http::request<boost::beast::http::string_body>& req, boost::system::error_code&),
                (override));
    MOCK_METHOD(boost::asio::awaitable<void>,
                async_write,
                (const boost::beast::http::request<boost::beast::http::string_body>&, boost::system::error_code&),
                (override));
    MOCK_METHOD(void,
                read,
                (boost::beast::flat_buffer&,
                 boost::beast::http::response<boost::beast::http::dynamic_body>&,
                 boost::system::error_code&),
                (override));
    MOCK_METHOD(boost::asio::awaitable<void>,
                async_read,
                (boost::beast::flat_buffer&,
                 boost::beast::http::response<boost::beast::http::dynamic_body>&,
                 boost::system::error_code&),
                (override));
    MOCK_METHOD(void, close, (), (override));
};
