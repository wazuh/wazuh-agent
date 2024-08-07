#include <http_client.hpp>

namespace http_client
{
    boost::beast::http::request<boost::beast::http::string_body>
    CreateHttpRequest(const boost::beast::http::verb method,
                      const std::string& url,
                      const std::string& host,
                      const std::string& token,
                      const std::string& body,
                      const std::string& user_pass)
    {
        boost::beast::http::request<boost::beast::http::string_body> req {method, url, 11};
        req.set(boost::beast::http::field::host, host);
        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(boost::beast::http::field::accept, "application/json");

        if (!token.empty())
        {
            req.set(boost::beast::http::field::authorization, "Bearer " + token);
        }

        if (!user_pass.empty())
        {
            req.set(boost::beast::http::field::authorization, "Basic " + user_pass);
        }

        if (!body.empty())
        {
            req.set(boost::beast::http::field::content_type, "application/json");
            req.body() = body;
            req.prepare_payload();
        }

        return req;
    }

} // namespace http_client
