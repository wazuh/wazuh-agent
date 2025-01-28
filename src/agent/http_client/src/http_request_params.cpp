#include <http_request_params.hpp>

#include <boost/url.hpp>

#include <logger.hpp>

namespace http_client
{
    HttpRequestParams::HttpRequestParams(MethodType method,
                                         const std::string& serverUrl,
                                         std::string endpoint,
                                         std::string userAgent,
                                         std::string verificationMode,
                                         std::string token,
                                         std::string userPass,
                                         std::string body)
        : Method(method)
        , Endpoint(std::move(endpoint))
        , User_agent(std::move(userAgent))
        , Verification_Mode(std::move(verificationMode))
        , Token(std::move(token))
        , User_pass(std::move(userPass))
        , Body(std::move(body))
    {
        const auto result = boost::urls::parse_uri(serverUrl);

        if (!result)
        {
            LogError("Invalid URL: {}. Error: {}", serverUrl, result.error().message());
            return;
        }

        const auto& url = *result;

        if (url.scheme() != "http" && url.scheme() != "https")
        {
            LogError("Invalid URL scheme: {}", serverUrl);
            return;
        }

        if (url.host().empty())
        {
            LogError("Invalid URL host: {}", serverUrl);
            return;
        }

        Use_Https = url.scheme() == "https";
        Host = url.host();
        Port = !url.port().empty() ? url.port() : (Use_Https ? "443" : "80");
    }

    bool HttpRequestParams::operator==(const HttpRequestParams& other) const
    {
        return Method == other.Method && Host == other.Host && Port == other.Port && Endpoint == other.Endpoint &&
               User_agent == other.User_agent && Verification_Mode == other.Verification_Mode && Token == other.Token &&
               User_pass == other.User_pass && Body == other.Body && Use_Https == other.Use_Https;
    }
} // namespace http_client
