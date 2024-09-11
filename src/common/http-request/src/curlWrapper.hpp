/*
 * Wazuh shared modules utils
 * Copyright (C) 2015, Wazuh Inc.
 * July 18, 2022.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _CURL_WRAPPER_HPP
#define _CURL_WRAPPER_HPP

#include "ICURLHandler.hpp"
#include "IRequestImplementator.hpp"
#include "curlHandlerCache.hpp"
#include "curlMultiHandler.hpp"
#include "curlSingleHandler.hpp"
#include "customDeleter.hpp"
#include <algorithm>
#include <atomic>
#include <curl/curl.h>
#include <map>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <thread>

static const std::map<OPTION_REQUEST_TYPE, CURLoption> OPTION_REQUEST_TYPE_MAP = {
    {OPT_URL, CURLOPT_URL},
    {OPT_CAINFO, CURLOPT_CAINFO},
    {OPT_TIMEOUT, CURLOPT_TIMEOUT},
    {OPT_WRITEDATA, CURLOPT_WRITEDATA},
    {OPT_USERAGENT, CURLOPT_USERAGENT},
    {OPT_POSTFIELDS, CURLOPT_POSTFIELDS},
    {OPT_WRITEFUNCTION, CURLOPT_WRITEFUNCTION},
    {OPT_POSTFIELDSIZE, CURLOPT_POSTFIELDSIZE},
    {OPT_CUSTOMREQUEST, CURLOPT_CUSTOMREQUEST},
    {OPT_UNIX_SOCKET_PATH, CURLOPT_UNIX_SOCKET_PATH},
    {OPT_FAILONERROR, CURLOPT_FAILONERROR},
    {OPT_FOLLOW_REDIRECT, CURLOPT_FOLLOWLOCATION},
    {OPT_MAX_REDIRECTIONS, CURLOPT_MAXREDIRS},
    {OPT_VERIFYPEER, CURLOPT_SSL_VERIFYPEER},
    {OPT_SSL_CERT, CURLOPT_SSLCERT},
    {OPT_SSL_KEY, CURLOPT_SSLKEY},
    {OPT_BASIC_AUTH, CURLOPT_USERPWD}};

auto constexpr MAX_REDIRECTIONS {20l};

/**
 * @brief This class is a wrapper of the curl library.
 */
class cURLWrapper final : public IRequestImplementator
{
private:
    using deleterCurlStringList = CustomDeleter<decltype(&curl_slist_free_all), curl_slist_free_all>;
    std::unique_ptr<curl_slist, deleterCurlStringList> m_curlHeaders;
    std::string m_returnValue;
    std::shared_ptr<ICURLHandler> m_curlHandler;

    static size_t writeData(char* data, size_t size, size_t nmemb, void* userdata)
    {
        const auto str {reinterpret_cast<std::string*>(userdata)};
        str->append(data, size * nmemb);
        return size * nmemb;
    }

    /**
     * @brief Get the cURL Handler object.
     *
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the cURL handler.
     * @return std::shared_ptr<ICURLHandler>
     */
    std::shared_ptr<ICURLHandler> curlHandlerInit(CurlHandlerTypeEnum handlerType,
                                                  const std::atomic<bool>& shouldRun = true)
    {
        return cURLHandlerCache::instance().getCurlHandler(handlerType, shouldRun);
    }

public:
    /**
     * @brief Create a cURLWrapper.
     *
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the handler.
     */
    cURLWrapper(CurlHandlerTypeEnum handlerType = CurlHandlerTypeEnum::SINGLE,
                const std::atomic<bool>& shouldRun = true)
    {
        m_curlHandler = curlHandlerInit(handlerType, shouldRun);

        if (!m_curlHandler || !m_curlHandler->getHandler())
        {
            throw std::runtime_error("cURL initialization failed");
        }

        this->setOption(OPT_WRITEFUNCTION, reinterpret_cast<void*>(cURLWrapper::writeData));

        this->setOption(OPT_WRITEDATA, &m_returnValue);

        this->setOption(OPT_FAILONERROR, 1l);

        this->setOption(OPT_FOLLOW_REDIRECT, 1l);

        this->setOption(OPT_MAX_REDIRECTIONS, MAX_REDIRECTIONS);
    }

    virtual ~cURLWrapper() = default;

    /**
     * @brief This method returns the value of the last request.
     * @return The value of the last request.
     */
    inline const std::string response() override
    {
        return m_returnValue;
    }

    /**
     * @brief This method sets an option to the curl handler.
     * @param optIndex The option index.
     * @param ptr The option value.
     */
    void setOption(const OPTION_REQUEST_TYPE optIndex, void* ptr) override
    {
        auto ret = curl_easy_setopt(m_curlHandler->getHandler().get(), OPTION_REQUEST_TYPE_MAP.at(optIndex), ptr);

        if (ret != CURLE_OK)
        {
            throw std::runtime_error("cURL set option failed");
        }
    }

    /**
     * @brief This method sets an option to the curl handler.
     * @param optIndex The option index.
     * @param opt The option value.
     */
    void setOption(const OPTION_REQUEST_TYPE optIndex, const std::string& opt) override
    {
        auto ret =
            curl_easy_setopt(m_curlHandler->getHandler().get(), OPTION_REQUEST_TYPE_MAP.at(optIndex), opt.c_str());

        if (ret != CURLE_OK)
        {
            throw std::runtime_error("cURLWrapper::setOption() failed");
        }
    }

    /**
     * @brief This method sets an option to the curl handler.
     * @param optIndex The option index.
     * @param opt The option value.
     */
    void setOption(const OPTION_REQUEST_TYPE optIndex, const long opt) override
    {
        auto ret = curl_easy_setopt(m_curlHandler->getHandler().get(), OPTION_REQUEST_TYPE_MAP.at(optIndex), opt);

        if (ret != CURLE_OK)
        {
            throw std::runtime_error("cURLWrapper::setOption() failed");
        }
    }

    /**
     * @brief This method adds an header to the curl handler.
     * @param header The header to be added.
     */
    void appendHeader(const std::string& header) override
    {
        if (!m_curlHeaders)
        {
            m_curlHeaders.reset(curl_slist_append(m_curlHeaders.get(), header.c_str()));
        }
        else
        {
            curl_slist_append(m_curlHeaders.get(), header.c_str());
        }
    }

    /**
     * @brief This method performs the request.
     */
    void execute() override
    {
        CURLcode setOptResult =
            curl_easy_setopt(m_curlHandler->getHandler().get(), CURLOPT_HTTPHEADER, m_curlHeaders.get());
        if (CURLE_OK != setOptResult)
        {
            throw std::runtime_error("cURLWrapper::execute() failed: Couldn't set HTTP headers");
        }

        m_curlHandler->execute();
    }
};

#endif // _CURL_WRAPPER_HPP
