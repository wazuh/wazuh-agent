#pragma once

#include <boost/asio.hpp>

#include <string>

namespace http_client
{
    class IHttpResolver
    {
    public:
        virtual ~IHttpResolver() = default;

        virtual boost::asio::ip::tcp::resolver::results_type Resolve(const std::string& host,
                                                                     const std::string& port) = 0;

        virtual boost::asio::awaitable<boost::asio::ip::tcp::resolver::results_type>
        AsyncResolve(const std::string& host, const std::string& port) = 0;
    };
} // namespace http_client
