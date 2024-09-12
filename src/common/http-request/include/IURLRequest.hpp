/*
 * Wazuh shared modules utils
 * Copyright (C) 2015, Wazuh Inc.
 * July 11, 2022.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _URL_REQUEST_HPP
#define _URL_REQUEST_HPP

#include <nlohmann/json.hpp>
#include "secureCommunication.hpp"
#include <atomic>
#include <functional>
#include <string>
#include <unordered_set>

enum SOCKET_TYPE
{
    SOCKET_UNIX,
    SOCKET_TCP
};

enum class CurlHandlerTypeEnum
{
    SINGLE,
    MULTI
};

// HTTP headers used by default in queries.
const std::unordered_set<std::string> DEFAULT_HEADERS {
    "Content-Type: application/json", "Accept: application/json", "Accept-Charset: utf-8"};

/**
 * @brief This class is an abstraction of URL.
 * It is a base class to store the type/configuration of the request to made.
 */
class URL
{
public:
    virtual ~URL() = default;
    /**
     * @brief Returns the socket path.
     * @return Socket path.
     */
    std::string unixSocketPath() const
    {
        return m_sock;
    }

    /**
     * @brief Returns the URL host.
     * @return URL host.
     */
    std::string url() const
    {
        return m_url;
    };

    /**
     * @brief Returns the socket type.
     * @return Socket type.
     */
    SOCKET_TYPE socketType() const
    {
        return m_socketType;
    };

protected:
    /**
     * @brief Variable to store the socket type.
     */
    SOCKET_TYPE m_socketType;
    /**
     * @brief Variable to store the socket URL.
     */
    std::string m_url;
    /**
     * @brief Variable to store the socket path.
     */
    std::string m_sock;
};

/**
 * @brief This class is used to configure a HTTP Unix socket.
 */
class HttpUnixSocketURL final : public URL
{
public:
    /**
     * @brief Constructor for HttpUnixSocketURL class.

     * @param sock Unix socket path.
     * @param url Socket URL.
     */
    HttpUnixSocketURL(const std::string& sock, const std::string& url)
    {
        m_socketType = SOCKET_UNIX;
        m_sock = sock;
        m_url = url;
    }
};

/**
 * @brief This class is used to configure a TCP socket.
 */
class HttpURL final : public URL
{
public:
    /**
     * @brief Constructor for HttpURL class.
     * @param url Socket URL.
     */
    HttpURL(const std::string& url)
    {
        m_socketType = SOCKET_TCP;
        m_url = url;
    }
};

/**
 * @brief This class is an interface to perform URL requests.
 * It provides a simple interface to send HTTP requests.
 */
class IURLRequest
{
public:
    virtual ~IURLRequest() = default;
    /**
     * @brief Virtual method to download a file from a URL.
     * @param url URL to send the request.
     * @param fileName File name to save the response.
     * @param onError Callback to be called when an error occurs.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Secure communication object.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    virtual void download(
        const URL& url,
        const std::string& fileName,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::unordered_set<std::string>& httpHeaders = {},
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true) = 0;

    /**
     * @brief Virtual method to send a POST request to a URL.
     * @param url URL to send the request.
     * @param data Data to send (nlohmann::json).
     * @param onSuccess Callback to be called when the request is successful.
     * @param onError Callback to be called when an error occurs.
     * @param fileName File name of output file.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Secure communication object.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    virtual void post(
        const URL& url,
        const nlohmann::json& data,
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::string& fileName = "",
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true) = 0;

    /**
     * @brief Virtual method to send a POST request to a URL.
     * @param url URL to send the request.
     * @param data Data to send (string).
     * @param onSuccess Callback to be called when the request is successful.
     * @param onError Callback to be called when an error occurs.
     * @param fileName File name of output file.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Secure communication object.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    virtual void post(
        const URL& url,
        const std::string& data,
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::string& fileName = "",
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true) = 0;

    /**
     * @brief Virtual method to send a GET request to a URL.
     * @param url URL to send the request.
     * @param onSuccess Callback to be called when the request is successful.
     * @param onError Callback to be called when an error occurs.
     * @param fileName File name of output file.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Secure communication object.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    virtual void get(
        const URL& url,
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::string& fileName = "",
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true) = 0;

    /**
     * @brief Virtual method to send a UPDATE request to a URL.
     * @param url URL to send the request.
     * @param data Data to sendi (nlohmann::json).
     * @param onSuccess Callback to be called when the request is successful.
     * @param onError Callback to be called when an error occurs.
     * @param fileName File name of output file.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Secure communication object.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    virtual void put(
        const URL& url,
        const nlohmann::json& data,
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::string& fileName = "",
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true) = 0;

    /**
     * @brief Virtual method to send a UPDATE request to a URL.
     * @param url URL to send the request.
     * @param data Data to send (string).
     * @param onSuccess Callback to be called when the request is successful.
     * @param onError Callback to be called when an error occurs.
     * @param fileName File name of output file.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Secure communication object.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    virtual void put(
        const URL& url,
        const std::string& data,
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::string& fileName = "",
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true) = 0;

    /**
     * @brief Virtual method to send a PATCH request to a URL.
     *
     * @param url URL to send the request.
     * @param data Data to send (nlohmann::json).
     * @param onSuccess Callback to be called when the request is successful.
     * @param onError Callback to be called when an error occurs.
     * @param fileName File name of output file.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Secure communication object.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    virtual void patch(
        const URL& url,
        const nlohmann::json& data,
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::string& fileName = "",
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true) = 0;

    /**
     * @brief Virtual method to send a PATCH request to a URL.
     *
     * @param url URL to send the request.
     * @param data Data to send (string).
     * @param onSuccess Callback to be called when the request is successful.
     * @param onError Callback to be called when an error occurs.
     * @param fileName File name of output file.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Secure communication object.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    virtual void patch(
        const URL& url,
        const std::string& data,
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::string& fileName = "",
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true) = 0;

    /**
     * @brief Virtual method to send a DELETE request to a URL.
     * @param url URL to send the request.
     * @param onSuccess Callback to be called when the request is successful.
     * @param onError Callback to be called when an error occurs.
     * @param fileName File name of output file.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Secure communication object.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    virtual void delete_(
        const URL& url,
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::string& fileName = "",
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true) = 0;
};

#endif // _URL_REQUEST_HPP
