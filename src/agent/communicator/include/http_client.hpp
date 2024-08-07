#pragma once

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

} // namespace http_client
