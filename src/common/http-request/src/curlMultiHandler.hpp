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

#ifndef _CURL_MULTI_HANDLER_HPP
#define _CURL_MULTI_HANDLER_HPP

#include "ICURLHandler.hpp"
#include "customDeleter.hpp"
#include <atomic>
#include <memory>
#include <stdexcept>

static const int CURL_MULTI_HANDLER_TIMEOUT_MS = 1000;
static const int CURL_MULTI_HANDLER_EXTRA_FDS = 0;

using deleterCurlHandler = CustomDeleter<decltype(&curl_easy_cleanup), curl_easy_cleanup>;
using deleterCurlMultiHandler = CustomDeleter<decltype(&curl_multi_cleanup), curl_multi_cleanup>;

//! cURLMultiHandler class
/**
 * @brief This class implements the ICURLHandler interface to represent a multi cURL handler.
 *
 */
class cURLMultiHandler final : public ICURLHandler
{
private:
    std::shared_ptr<CURLM> m_curlMultiHandler; ///< Pointer to the cURL multi handler.
    const std::atomic<bool>& m_shouldRun;      ///< Variable to control the graceful shutdown of the cURL multi handler.

public:
    /**
     * @brief Construct a new cURLMultiHandler object
     *
     * @param curlHandlerType Enum value of the cURL handler.
     * @param shouldRun Flag used to interrupt the cURL handler.
     */
    explicit cURLMultiHandler(CurlHandlerTypeEnum curlHandlerType, const std::atomic<bool>& shouldRun = true)
        : ICURLHandler(curlHandlerType)
        , m_shouldRun(shouldRun)
    {
        m_curlHandler = std::shared_ptr<CURL>(curl_easy_init(), deleterCurlHandler());
        m_curlMultiHandler = std::shared_ptr<CURLM>(curl_multi_init(), deleterCurlMultiHandler());
    }

    // LCOV_EXCL_START
    ~cURLMultiHandler() override
    {
        curl_multi_remove_handle(m_curlMultiHandler.get(), m_curlHandler.get());
        curl_global_cleanup();
    }
    // LCOV_EXCL_STOP

    /**
     * @brief Performs the request using the curl multi-handler, the request execution can be canceled when the
     * 'm_shouldRun' variable is set to false by the class utilizing this method.
     *
     */
    void execute() override
    {
        int stillRunning {1};

        // Adds the single-handler to the multi-handler
        auto multiCode {curl_multi_add_handle(m_curlMultiHandler.get(), m_curlHandler.get())};

        if (multiCode != CURLM_OK)
        {
            throw std::runtime_error("cURLMultiHandler::execute() failed: curl_multi_add_handle");
        }

        do
        {
            // Performs transfers on the added single-handler
            multiCode = curl_multi_perform(m_curlMultiHandler.get(), &stillRunning);

            if (multiCode != CURLM_OK)
            {
                throw std::runtime_error("cURLMultiHandler::execute() failed: curl_multi_perform");
            }

            int fileDescriptors;

            // Waits until activity is detected or `CURL_MULTI_HANDLER_TIMEOUT_MS` has passed
            multiCode = curl_multi_wait(m_curlMultiHandler.get(),
                                        nullptr,
                                        CURL_MULTI_HANDLER_EXTRA_FDS,
                                        CURL_MULTI_HANDLER_TIMEOUT_MS,
                                        &fileDescriptors);
            if (multiCode != CURLM_OK)
            {
                throw std::runtime_error("cURLMultiHandler::execute() failed: curl_multi_wait");
            }
        } while (stillRunning && m_shouldRun.load());

        long responseCode;
        const auto resGetInfo {curl_easy_getinfo(m_curlHandler.get(), CURLINFO_RESPONSE_CODE, &responseCode)};

        if (resGetInfo != CURLE_OK)
        {
            throw std::runtime_error("cURLMultiHandler::execute() failed: Couldn't get HTTP response code");
        }
    }
};

#endif // _CURL_MULTI_HANDLER_HPP
