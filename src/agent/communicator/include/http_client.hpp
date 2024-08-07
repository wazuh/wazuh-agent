#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <string>

namespace http_client
{
    boost::beast::http::request<boost::beast::http::string_body>
    CreateHttpRequest(const boost::beast::http::verb method,
                      const std::string& url,
                      const std::string& host,
                      const std::string& token,
                      const std::string& body = "",
                      const std::string& user_pass = "");

    boost::asio::awaitable<void>
    Co_PerformHttpRequest(boost::asio::ip::tcp::socket& socket,
                          boost::beast::http::request<boost::beast::http::string_body>& req,
                          boost::beast::error_code& ec);


    boost::beast::http::response<boost::beast::http::dynamic_body>
    SendHttpRequest(const boost::beast::http::verb method,
                    const std::string& ip,
                    const std::string& port,
                    const std::string& url,
                    const std::string& token,
                    const std::string& body);
} // namespace http_client
