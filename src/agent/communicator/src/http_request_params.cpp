#include <http_request_params.hpp>

#include <boost/url.hpp>
#include <logger.hpp>

namespace http_client
{
    HttpRequestParams::HttpRequestParams(boost::beast::http::verb method,
                                         const std::string& serverUrl,
                                         std::string endpoint,
                                         std::string userAgent,
                                         std::string token,
                                         std::string userPass,
                                         std::string body)
        : Method(method)
        , Endpoint(std::move(endpoint))
        , User_agent(std::move(userAgent))
        , Token(std::move(token))
        , User_pass(std::move(userPass))
        , Body(std::move(body))
    {
        auto result = boost::urls::parse_uri(serverUrl);
        if (!result)
        {
            LogError("Invalid URL: {}", result.error().message());
            return;
        }

        boost::urls::url_view url = *result;
        Use_Https = url.scheme().empty() || url.scheme() == "https";
        Host = url.host();
        Port = url.port();
    }

    bool HttpRequestParams::operator==(const HttpRequestParams& other) const
    {
        return Method == other.Method && Host == other.Host && Port == other.Port && Endpoint == other.Endpoint &&
               User_agent == other.User_agent && Token == other.Token && User_pass == other.User_pass &&
               Body == other.Body && Use_Https == other.Use_Https;
    }
} // namespace http_client
