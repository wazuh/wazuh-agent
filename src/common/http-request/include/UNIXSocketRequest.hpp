/*
 * Wazuh shared modules utils
 * Copyright (C) 2015, Wazuh Inc.
 * July 12, 2022.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _UNIX_SOCKET_REQUEST_HPP
#define _UNIX_SOCKET_REQUEST_HPP

#include "IURLRequest.hpp"
#include <nlohmann/json.hpp>
#include "singleton.hpp"
#include <atomic>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_set>

/**
 * @brief This class is an abstraction of HTTP Unix socket request.
 * It provides a simple interface to send HTTP requests.
 */
class UNIXSocketRequest final
    : public IURLRequest
    , public Singleton<UNIXSocketRequest>
{
public:
    /**
     * @brief Performs a UNIX SOCKET DOWNLOAD request.
     *
     * @param url URL to send the request.
     * @param fileName File name of output file.
     * @param onError Callback to be called in case of error.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Object that provides secure communication.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    void download(
        const URL& url,
        const std::string& fileName,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true);
    /**
     * @brief Performs a UNIX SOCKET POST request.
     *
     * @param url URL to send the request.
     * @param data Data to send (nlohmann::json).
     * @param onSuccess Callback to be called in case of success.
     * @param onError Callback to be called in case of error.
     * @param fileName File name of output file.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Object that provides secure communication.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    void post(
        const URL& url,
        const nlohmann::json& data,
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::string& fileName = "",
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true);

    /**
     * @brief Performs a UNIX SOCKET POST request.
     *
     * @param url URL to send the request.
     * @param data Data to send (string).
     * @param onSuccess Callback to be called in case of success.
     * @param onError Callback to be called in case of error.
     * @param fileName File name of output file.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Object that provides secure communication.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    void post(
        const URL& url,
        const std::string& data,
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::string& fileName = "",
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true);
    /**
     * @brief Performs a UNIX SOCKET GET request.
     *
     * @param url URL to send the request.
     * @param onSuccess Callback to be called in case of success.
     * @param onError Callback to be called in case of error.
     * @param fileName File name of output file.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Object that provides secure communication.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    void get(
        const URL& url,
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::string& fileName = "",
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true);
    /**
     * @brief Performs a UNIX SOCKET PUT request.
     *
     * @param url URL to send the request.
     * @param data Data to send (nlohmann::json).
     * @param onSuccess Callback to be called in case of success.
     * @param onError Callback to be called in case of error.
     * @param fileName File name of output file.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Object that provides secure communication.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    void put(
        const URL& url,
        const nlohmann::json& data,
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::string& fileName = "",
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true);
    /**
     * @brief Performs a UNIX SOCKET PUT request.
     *
     * @param url URL to send the request.
     * @param data Data to send (string).
     * @param onSuccess Callback to be called in case of success.
     * @param onError Callback to be called in case of error.
     * @param fileName File name of output file.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Object that provides secure communication.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    void put(
        const URL& url,
        const std::string& data,
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::string& fileName = "",
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true);
    /**
     * @brief Performs a UNIX SOCKET PATCH request.
     *
     * @param url URL to send the request.
     * @param data Data to send (nlohmann::json).
     * @param onSuccess Callback to be called in case of success.
     * @param onError Callback to be called in case of error.
     * @param fileName File name of output file.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Object that provides secure communication.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    void patch(
        const URL& url,
        const nlohmann::json& data,
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::string& fileName = "",
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true);
    /**
     * @brief Performs a UNIX SOCKET PATCH request.
     *
     * @param url URL to send the request.
     * @param data Data to send (string).
     * @param onSuccess Callback to be called in case of success.
     * @param onError Callback to be called in case of error.
     * @param fileName File name of output file.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Object that provides secure communication.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    void patch(
        const URL& url,
        const std::string& data,
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::string& fileName = "",
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true);
    /**
     * @brief Performs a UNIX SOCKET DELETE request.
     *
     * @param url URL to send the request.
     * @param onSuccess Callback to be called in case of success.
     * @param onError Callback to be called in case of error.
     * @param fileName File name of output file.
     * @param httpHeaders Headers to be added to the query.
     * @param secureCommunication Object that provides secure communication.
     * @param userAgent User agent to be used in the request.
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler when the 'handlerType' is set to 'MULTI'.
     */
    void delete_(
        const URL& url,
        std::function<void(const std::string&)> onSuccess,
        std::function<void(const std::string&, const long)> onError = [](auto, auto) {},
        const std::string& fileName = "",
        const std::unordered_set<std::string>& httpHeaders = DEFAULT_HEADERS,
        const SecureCommunication& secureCommunication = {},
        const std::string& userAgent = {},
        const CurlHandlerTypeEnum& handlerType = CurlHandlerTypeEnum::SINGLE,
        const std::atomic<bool>& shouldRun = true);
};

#endif // _UNIX_SOCKET_REQUEST_HPP
