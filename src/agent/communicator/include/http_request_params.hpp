#pragma once

#include <boost/beast.hpp>

#include <string>

namespace http_client
{
    /// @struct HttpRequestParams
    /// @brief Parameters for HTTP requests
    struct HttpRequestParams
    {
        boost::beast::http::verb Method;
        std::string Host;
        std::string Port;
        std::string Endpoint;
        std::string User_agent;
        std::string Verification_Mode;
        std::string Token;
        std::string User_pass;
        std::string Body;
        bool Use_Https;

        /// @brief Constructs HttpRequestParams with specified parameters
        /// @param method The HTTP method to use
        /// @param serverUrl The server URL for the request
        /// @param endpoint The endpoint for the request
        /// @param userAgent The user agent property for the request header
        /// @param verificationMode The verification mode for the request
        /// @param token Optional token for authorization
        /// @param userPass Optional user credentials for basic authentication
        /// @param body Optional body for the request
        HttpRequestParams(boost::beast::http::verb method,
                          const std::string& serverUrl,
                          std::string endpoint,
                          std::string userAgent,
                          std::string verificationMode,
                          std::string token = "",
                          std::string userPass = "",
                          std::string body = "");

        /// @brief Equality operator for comparing two HttpRequestParams objects
        /// @param other The other HttpRequestParams object to compare with
        /// @return True if equal, false otherwise
        bool operator==(const HttpRequestParams& other) const;
    };
} // namespace http_client
