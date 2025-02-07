#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include <boost/system/error_code.hpp>

#include <chrono>

namespace http_client
{
    /// @brief Wrapper Interface for boost functions - To be replaced with mocks in unit tests
    class ISocketHelper
    {
    public:
        virtual ~ISocketHelper() {};

        virtual void set_verify_mode(boost::asio::ssl::verify_mode mode) = 0;

        virtual void set_verify_callback(std::function<bool(bool, boost::asio::ssl::verify_context&)> vf) = 0;

        virtual void expires_after(std::chrono::seconds seconds) = 0;

        virtual void connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                             boost::system::error_code& ec) = 0;

        virtual boost::asio::awaitable<void>
        async_connect(const boost::asio::ip::tcp::resolver::results_type& endpoints, boost::system::error_code& ec) = 0;

        virtual void write(const boost::beast::http::request<boost::beast::http::string_body>& req,
                           boost::system::error_code& ec) = 0;

        virtual boost::asio::awaitable<void>
        async_write(const boost::beast::http::request<boost::beast::http::string_body>& req,
                    boost::system::error_code& ec) = 0;

        virtual void read(boost::beast::flat_buffer& buffer,
                          boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                          boost::system::error_code& ec) = 0;

        virtual boost::asio::awaitable<void>
        async_read(boost::beast::flat_buffer& buffer,
                   boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                   boost::system::error_code& ec) = 0;

        virtual void close() = 0;
    };

} // namespace http_client
