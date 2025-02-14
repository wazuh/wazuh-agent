#pragma once

#include <ihttp_socket_wrapper.hpp>

#include <boost/asio.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/system/error_code.hpp>

namespace http_client
{
    /// @brief Helper class that wraps boost network functions for testing purposes
    class HttpSocketHelper : public ISocketWrapper
    {
    public:
        HttpSocketHelper(const boost::asio::any_io_executor& io_context)
            : m_socket(io_context)
        {
        }

        void set_verify_mode(boost::asio::ssl::verify_mode) override
        { // No implementation for Http
        }

        void set_verify_callback(std::function<bool(bool, boost::asio::ssl::verify_context&)>) override
        { // No implementation for Http
        }

        void expires_after(std::chrono::milliseconds ms) override
        {
            m_socket.expires_after(std::chrono::duration_cast<std::chrono::seconds>(ms));
        }

        void connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                     boost::system::error_code& ec) override
        {
            m_socket.async_connect(endpoints, [&ec](boost::system::error_code const& code, auto const&) { ec = code; });
        }

        boost::asio::awaitable<void> async_connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                   boost::system::error_code& ec) override
        {
            co_await m_socket.async_connect(endpoints, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        }

        void write(const boost::beast::http::request<boost::beast::http::string_body>& req,
                   boost::system::error_code& ec) override
        {
            boost::beast::http::write(m_socket, req, ec);
        }

        boost::asio::awaitable<void>
        async_write(const boost::beast::http::request<boost::beast::http::string_body>& req,
                    boost::system::error_code& ec) override
        {
            co_await boost::beast::http::async_write(
                m_socket, req, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        }

        void read(boost::beast::flat_buffer& buffer,
                  boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                  boost::system::error_code& ec) override
        {
            boost::beast::http::read(m_socket, buffer, res, ec);
        }

        boost::asio::awaitable<void> async_read(boost::beast::flat_buffer& buffer,
                                                boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                                                boost::system::error_code& ec) override
        {
            co_await boost::beast::http::async_read(
                m_socket, buffer, res, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        }

        void close() override
        {
            m_socket.close();
        }

    private:
        boost::beast::tcp_stream m_socket;
    };
} // namespace http_client
