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

#include "UNIXSocketRequest.hpp"
#include "factoryRequestImplemetator.hpp"
#include "urlRequest.hpp"
#include <atomic>
#include <string>
#include <unordered_set>

using wrapperType = cURLWrapper;

void UNIXSocketRequest::download(const URL& url,
                                 const std::string& outputFile,
                                 std::function<void(const std::string&, const long)> onError,
                                 const std::unordered_set<std::string>& httpHeaders,
                                 const SecureCommunication& secureCommunication,
                                 const std::string& userAgent,
                                 const CurlHandlerTypeEnum& handlerType,
                                 const std::atomic<bool>& shouldRun)
{
    try
    {
        GetRequest::builder(FactoryRequestWrapper<wrapperType>::create(handlerType, shouldRun))
            .url(url.url(), secureCommunication)
            .unixSocketPath(url.unixSocketPath())
            .userAgent(userAgent)
            .outputFile(outputFile)
            .execute();
    }
    catch (const Curl::CurlException& ex)
    {
        onError(ex.what(), ex.responseCode());
    }
    catch (const std::exception& ex)
    {
        onError(ex.what(), NOT_USED);
    }
}

void UNIXSocketRequest::post(const URL& url,
                             const nlohmann::json& data,
                             std::function<void(const std::string&)> onSuccess,
                             std::function<void(const std::string&, const long)> onError,
                             const std::string& fileName,
                             const std::unordered_set<std::string>& httpHeaders,
                             const SecureCommunication& secureCommunication,
                             const std::string& userAgent,
                             const CurlHandlerTypeEnum& handlerType,
                             const std::atomic<bool>& shouldRun)
{
    std::string dataStr;
    try
    {
        dataStr = data.dump();
    }
    catch (const std::exception& ex)
    {
        onError(ex.what(), NOT_USED);
        return;
    }
    post(url, dataStr, std::move(onSuccess), std::move(onError), fileName, httpHeaders, secureCommunication, userAgent);
}

void UNIXSocketRequest::post(const URL& url,
                             const std::string& data,
                             std::function<void(const std::string&)> onSuccess,
                             std::function<void(const std::string&, const long)> onError,
                             const std::string& fileName,
                             const std::unordered_set<std::string>& httpHeaders,
                             const SecureCommunication& secureCommunication,
                             const std::string& userAgent,
                             const CurlHandlerTypeEnum& handlerType,
                             const std::atomic<bool>& shouldRun)
{
    try
    {
        auto req {PostRequest::builder(FactoryRequestWrapper<wrapperType>::create(handlerType, shouldRun))};
        req.url(url.url(), secureCommunication)
            .unixSocketPath(url.unixSocketPath())
            .userAgent(userAgent)
            .postData(data)
            .outputFile(fileName)
            .execute();

        onSuccess(req.response());
    }
    catch (const Curl::CurlException& ex)
    {
        onError(ex.what(), ex.responseCode());
    }
    catch (const std::exception& ex)
    {
        onError(ex.what(), NOT_USED);
    }
}

void UNIXSocketRequest::get(const URL& url,
                            std::function<void(const std::string&)> onSuccess,
                            std::function<void(const std::string&, const long)> onError,
                            const std::string& fileName,
                            const std::unordered_set<std::string>& httpHeaders,
                            const SecureCommunication& secureCommunication,
                            const std::string& userAgent,
                            const CurlHandlerTypeEnum& handlerType,
                            const std::atomic<bool>& shouldRun)
{
    try
    {
        auto req {GetRequest::builder(FactoryRequestWrapper<wrapperType>::create(handlerType, shouldRun))};
        req.url(url.url(), secureCommunication)
            .unixSocketPath(url.unixSocketPath())
            .userAgent(userAgent)
            .outputFile(fileName)
            .execute();

        onSuccess(req.response());
    }
    catch (const Curl::CurlException& ex)
    {
        onError(ex.what(), ex.responseCode());
    }
    catch (const std::exception& ex)
    {
        onError(ex.what(), NOT_USED);
    }
}

void UNIXSocketRequest::put(const URL& url,
                            const nlohmann::json& data,
                            std::function<void(const std::string&)> onSuccess,
                            std::function<void(const std::string&, const long)> onError,
                            const std::string& fileName,
                            const std::unordered_set<std::string>& httpHeaders,
                            const SecureCommunication& secureCommunication,
                            const std::string& userAgent,
                            const CurlHandlerTypeEnum& handlerType,
                            const std::atomic<bool>& shouldRun)
{
    std::string dataStr;
    try
    {
        dataStr = data.dump();
    }
    catch (const std::exception& ex)
    {
        onError(ex.what(), NOT_USED);
        return;
    }
    put(url, dataStr, std::move(onSuccess), std::move(onError), fileName, httpHeaders, secureCommunication, userAgent);
}

void UNIXSocketRequest::put(const URL& url,
                            const std::string& data,
                            std::function<void(const std::string&)> onSuccess,
                            std::function<void(const std::string&, const long)> onError,
                            const std::string& fileName,
                            const std::unordered_set<std::string>& httpHeaders,
                            const SecureCommunication& secureCommunication,
                            const std::string& userAgent,
                            const CurlHandlerTypeEnum& handlerType,
                            const std::atomic<bool>& shouldRun)
{
    try
    {
        auto req {PutRequest::builder(FactoryRequestWrapper<wrapperType>::create(handlerType, shouldRun))};
        req.url(url.url(), secureCommunication)
            .unixSocketPath(url.unixSocketPath())
            .userAgent(userAgent)
            .postData(data)
            .outputFile(fileName)
            .execute();

        onSuccess(req.response());
    }
    catch (const Curl::CurlException& ex)
    {
        onError(ex.what(), ex.responseCode());
    }
    catch (const std::exception& ex)
    {
        onError(ex.what(), NOT_USED);
    }
}

void UNIXSocketRequest::patch(const URL& url,
                              const nlohmann::json& data,
                              std::function<void(const std::string&)> onSuccess,
                              std::function<void(const std::string&, const long)> onError,
                              const std::string& fileName,
                              const std::unordered_set<std::string>& httpHeaders,
                              const SecureCommunication& secureCommunication,
                              const std::string& userAgent,
                              const CurlHandlerTypeEnum& handlerType,
                              const std::atomic<bool>& shouldRun)
{
    std::string dataStr;
    try
    {
        dataStr = data.dump();
    }
    catch (const std::exception& ex)
    {
        onError(ex.what(), NOT_USED);
        return;
    }
    patch(
        url, dataStr, std::move(onSuccess), std::move(onError), fileName, httpHeaders, secureCommunication, userAgent);
}

void UNIXSocketRequest::patch(const URL& url,
                              const std::string& data,
                              std::function<void(const std::string&)> onSuccess,
                              std::function<void(const std::string&, const long)> onError,
                              const std::string& fileName,
                              const std::unordered_set<std::string>& httpHeaders,
                              const SecureCommunication& secureCommunication,
                              const std::string& userAgent,
                              const CurlHandlerTypeEnum& handlerType,
                              const std::atomic<bool>& shouldRun)
{
    try
    {
        auto req {PatchRequest::builder(FactoryRequestWrapper<wrapperType>::create(handlerType, shouldRun))};
        req.url(url.url(), secureCommunication)
            .unixSocketPath(url.unixSocketPath())
            .userAgent(userAgent)
            .postData(data)
            .outputFile(fileName)
            .execute();

        onSuccess(req.response());
    }
    catch (const Curl::CurlException& ex)
    {
        onError(ex.what(), ex.responseCode());
    }
    catch (const std::exception& ex)
    {
        onError(ex.what(), NOT_USED);
    }
}

void UNIXSocketRequest::delete_(const URL& url,
                                std::function<void(const std::string&)> onSuccess,
                                std::function<void(const std::string&, const long)> onError,
                                const std::string& fileName,
                                const std::unordered_set<std::string>& httpHeaders,
                                const SecureCommunication& secureCommunication,
                                const std::string& userAgent,
                                const CurlHandlerTypeEnum& handlerType,
                                const std::atomic<bool>& shouldRun)
{
    try
    {
        auto req {DeleteRequest::builder(FactoryRequestWrapper<cURLWrapper>::create())};
        req.url(url.url(), secureCommunication)
            .unixSocketPath(url.unixSocketPath())
            .userAgent(userAgent)
            .outputFile(fileName)
            .execute();

        onSuccess(req.response());
    }
    catch (const Curl::CurlException& ex)
    {
        onError(ex.what(), ex.responseCode());
    }
    catch (const std::exception& ex)
    {
        onError(ex.what(), NOT_USED);
    }
}
