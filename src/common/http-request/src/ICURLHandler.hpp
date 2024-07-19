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

#ifndef _CURL_HANDLER_HPP
#define _CURL_HANDLER_HPP

#include <curl/curl.h>
#include <memory>

//! ICURLHandler abstract class
/**
 * @brief This class serves as the interface that represents a cURL handler.
 *
 */
class ICURLHandler
{
protected:
    std::shared_ptr<CURL> m_curlHandler;         ///< Pointer to the CURL handle.
    const CurlHandlerTypeEnum m_curlHandlerType; ///< Enum value for this cURL handler.

public:
    /**
     * @brief Construct a new ICURLHandler object
     *
     * @param curlHandlerType Enum value of the cURL handler.
     */
    explicit ICURLHandler(CurlHandlerTypeEnum curlHandlerType)
        : m_curlHandlerType(curlHandlerType) {};

    // LCOV_EXCL_START
    virtual ~ICURLHandler() = default;
    // LCOV_EXCL_STOP

    /**
     * @brief Performs the request.
     */
    virtual void execute() = 0;

    /**
     * @brief Returns the pointer to the CURL handle.
     *
     * @return std::shared_ptr<CURL> cURL handler object.
     */
    [[nodiscard]] const std::shared_ptr<CURL>& getHandler() const
    {
        return m_curlHandler;
    }

    /**
     * @brief Returns the type of the cURL handler.
     *
     * @return CurlHandlerTypeEnum Enum value of the cURL handler type.
     */
    [[nodiscard]] CurlHandlerTypeEnum getHandlerType() const
    {
        return m_curlHandlerType;
    }
};

#endif // _CURL_HANDLER_HPP
