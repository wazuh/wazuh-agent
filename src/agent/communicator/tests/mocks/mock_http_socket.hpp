#include <gmock/gmock.h>

#include <ihttp_socket.hpp>

class MockHttpSocket : public http_client::IHttpSocket
{
public:
    MOCK_METHOD(void, SetVerificationMode, (const std::string& host, const std::string& verificationMode), (override));
    MOCK_METHOD(void,
                Connect,
                (const boost::asio::ip::tcp::resolver::results_type& endpoints, boost::system::error_code& code),
                (override));

    MOCK_METHOD(boost::asio::awaitable<void>,
                AsyncConnect,
                (const boost::asio::ip::tcp::resolver::results_type& endpoints, boost::system::error_code& code),
                (override));

    MOCK_METHOD(void,
                Write,
                (const boost::beast::http::request<boost::beast::http::string_body>& req,
                 boost::system::error_code& ec),
                (override));

    MOCK_METHOD(boost::asio::awaitable<void>,
                AsyncWrite,
                (const boost::beast::http::request<boost::beast::http::string_body>& req,
                 boost::system::error_code& ec),
                (override));

    MOCK_METHOD(void,
                Read,
                (boost::beast::http::response<boost::beast::http::dynamic_body> & res, boost::system::error_code& ec),
                (override));

    MOCK_METHOD(boost::asio::awaitable<void>,
                AsyncRead,
                (boost::beast::http::response<boost::beast::http::dynamic_body> & res, boost::system::error_code& ec),
                (override));

    MOCK_METHOD(void, Close, (), (override));
};
