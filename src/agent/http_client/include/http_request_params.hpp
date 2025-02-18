#pragma once

#include <string>

namespace http_client
{
    /// @brief HTTP status codes
    constexpr int HTTP_CODE_OK = 200;
    constexpr int HTTP_CODE_CREATED = 201;
    constexpr int HTTP_CODE_MULTIPLE_CHOICES = 300;
    constexpr int HTTP_CODE_BAD_REQUEST = 400;
    constexpr int HTTP_CODE_UNAUTHORIZED = 401;
    constexpr int HTTP_CODE_FORBIDDEN = 403;
    constexpr int HTTP_CODE_TIMEOUT = 408;
    constexpr int HTTP_CODE_INTERNAL_SERVER_ERROR = 500;

    /// @brief Supported HTTP methods
    enum class MethodType
    {
        GET,
        POST,
        PUT,
        DELETE_
    };

    /// @struct HttpRequestParams
    /// @brief Parameters for HTTP requests
    struct HttpRequestParams
    {
        MethodType Method;
        std::string Host;
        std::string Port;
        std::string Endpoint;
        std::string User_agent;
        std::string Verification_Mode;
        std::string Token;
        std::string User_pass;
        std::string Body;
        bool Use_Https;
        time_t RequestTimeout;

        /// @brief Constructs HttpRequestParams with specified parameters
        /// @param method The HTTP method to use
        /// @param serverUrl The server URL for the request
        /// @param endpoint The endpoint for the request
        /// @param userAgent The user agent property for the request header
        /// @param verificationMode The verification mode for the request
        /// @param token Optional token for authorization
        /// @param userPass Optional user credentials for basic authentication
        /// @param body Optional body for the request
        /// @param requestTimeoutInMilliSeconds Optional request timeout in milliseconds
        HttpRequestParams(MethodType method,
                          const std::string& serverUrl,
                          std::string endpoint,
                          std::string userAgent = "",
                          std::string verificationMode = "none",
                          std::string token = "",
                          std::string userPass = "",
                          std::string body = "",
                          const time_t requestTimeoutInMilliSeconds = 0);

        /// @brief Equality operator for comparing two HttpRequestParams objects
        /// @param other The other HttpRequestParams object to compare with
        /// @return True if equal, false otherwise
        bool operator==(const HttpRequestParams& other) const;
    };
} // namespace http_client
