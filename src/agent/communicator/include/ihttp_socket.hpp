#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace http_client
{
    class IHttpSocket
    {
    public:
        virtual ~IHttpSocket() = default;

        virtual void Connect(const boost::asio::ip::tcp::resolver::results_type& endpoints) = 0;

        virtual boost::asio::awaitable<void> AsyncConnect(const boost::asio::ip::tcp::resolver::results_type& endpoints,
                                                          boost::system::error_code& code) = 0;

        virtual void Write(const boost::beast::http::request<boost::beast::http::string_body>& req) = 0;

        virtual boost::asio::awaitable<void>
        AsyncWrite(const boost::beast::http::request<boost::beast::http::string_body>& req,
                   boost::beast::error_code& ec) = 0;

        virtual void Read(boost::beast::http::response<boost::beast::http::dynamic_body>& res) = 0;

        virtual void ReadToFile(boost::beast::http::response_parser<boost::beast::http::dynamic_body>& res,
                                const std::string& dstFilePath) = 0;

        virtual boost::asio::awaitable<void>
        AsyncRead(boost::beast::http::response<boost::beast::http::dynamic_body>& res,
                  boost::beast::error_code& ec) = 0;

        virtual void Close() = 0;
    };
} // namespace http_client
