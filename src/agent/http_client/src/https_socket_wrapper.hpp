#pragma once

#include <ihttp_socket_wrapper.hpp>

#include <logger.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/system/error_code.hpp>

namespace http_client
{
    /// @brief Helper class that wraps boost network functions for testing purposes
    class HttpsSocketHelper : public ISocketWrapper
    {
    public:
        HttpsSocketHelper(const boost::asio::any_io_executor& io_context, boost::asio::ssl::context& ctx)
            : m_socket(io_context, ctx)
        {
        }

        void set_verify_mode(boost::asio::ssl::verify_mode mode) override
        {
            m_socket.set_verify_mode(mode);
        }

        void set_verify_callback(std::function<bool(bool, boost::asio::ssl::verify_context&)> vf) override
        {
            m_socket.set_verify_callback(vf);
        }

        void expires_after(std::chrono::milliseconds ms) override
        {
            m_socket.next_layer().expires_after(ms);
        }

        void connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                     boost::system::error_code& ec) override
        {
            m_socket.next_layer().async_connect(endpoints,
                                                [this, &ec](boost::system::error_code const& ecConnect, auto const&)
                                                {
                                                    ec = ecConnect;
                                                    if (ec)
                                                    {
                                                        LogDebug("Connect failed: {}", ec.message());
                                                        return;
                                                    }

                                                    m_socket.async_handshake(
                                                        boost::asio::ssl::stream_base::client,
                                                        [&ec](const boost::system::error_code& ecHandshake)
                                                        {
                                                            ec = ecHandshake;
                                                            if (ecHandshake)
                                                            {
                                                                LogDebug("Handshake failed: {}", ecHandshake.message());
                                                            }
                                                        });
                                                });
        }

        boost::asio::awaitable<void> async_connect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                   boost::system::error_code& ec) override
        {
            co_await m_socket.next_layer().async_connect(endpoints,
                                                         boost::asio::redirect_error(boost::asio::use_awaitable, ec));

            if (ec)
            {
                LogDebug("boost::asio::async_connect returned error code: {} {}", ec.value(), ec.message());
            }
            else
            {
                co_await m_socket.async_handshake(boost::asio::ssl::stream_base::client,
                                                  boost::asio::redirect_error(boost::asio::use_awaitable, ec));
                if (ec)
                {
                    LogDebug("boost::asio::async_handshake returned error code: {} {}", ec.value(), ec.message());
                }
            }
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
            m_socket.shutdown();
        }

    private:
        boost::beast::ssl_stream<boost::beast::tcp_stream> m_socket;
    };
} // namespace http_client
