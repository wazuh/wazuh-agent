/*
 * Wazuh shared modules utils
 * Copyright (C) 2015, Wazuh Inc.
 * December 27, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _CURL_SINGLE_HANDLER_HPP
#define _CURL_SINGLE_HANDLER_HPP

#include "ICURLHandler.hpp"
#include "curlException.hpp"
#include "customDeleter.hpp"
#include <curl/curl.h>
#include <memory>
#include <stdexcept>
#include <utility>

using deleterCurlHandler = CustomDeleter<decltype(&curl_easy_cleanup), curl_easy_cleanup>;

//! cURLSingleHandler class
/**
 * @brief class implements the ICURLHandler interface to represent a single cURL handler.
 */
class cURLSingleHandler final : public ICURLHandler
{
public:
    /**
     * @brief Construct a new cURLSingleHandler object
     *
     * @param curlHandlerType Enum value of the cURL handler.
     */
    explicit cURLSingleHandler(CurlHandlerTypeEnum curlHandlerType)
        : ICURLHandler(curlHandlerType)
    {
        m_curlHandler = std::shared_ptr<CURL>(curl_easy_init(), deleterCurlHandler());
    }

    // LCOV_EXCL_START
    ~cURLSingleHandler() override = default;
    // LCOV_EXCL_STOP

    /**
     * @brief This method performs the request.
     */
    void execute() override
    {
        const auto resPerform {curl_easy_perform(m_curlHandler.get())};

        long responseCode;
        const auto resGetInfo {curl_easy_getinfo(m_curlHandler.get(), CURLINFO_RESPONSE_CODE, &responseCode)};

        curl_easy_reset(m_curlHandler.get());

        if (resPerform != CURLE_OK)
        {
            if (resPerform == CURLE_HTTP_RETURNED_ERROR)
            {
                if (resGetInfo != CURLE_OK)
                {
                    throw std::runtime_error("cURLSingleHandler::execute() failed: Couldn't get HTTP response code");
                }
                throw Curl::CurlException(curl_easy_strerror(resPerform), responseCode);
            }
            throw std::runtime_error(curl_easy_strerror(resPerform));
        }
    }
};

#endif // _CURL_SINGLE_HANDLER_HPP
