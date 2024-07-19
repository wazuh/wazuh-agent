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

#ifndef _FACTORY_REQUEST_WRAPPER_HPP
#define _FACTORY_REQUEST_WRAPPER_HPP

#include "IRequestImplementator.hpp"
#include "curlWrapper.hpp"
#include <atomic>
#include <memory>
#include <stdexcept>

/**
 * @brief This class is a factory for IRequestImplementator.
 * It provides a simple interface to create a IRequestImplementator.
 *
 * @tparam T Type of the response body.
 */
template<class Type>
class FactoryRequestWrapper final
{
public:
    /**
     * @brief Templatized factory pattern.
     */
    static std::shared_ptr<IRequestImplementator> create()
    {
        throw std::runtime_error("Request url initialization failed");
    }
};

/**
 * @brief This class is a specialization of FactoryRequestWrapper for cURLRequest.
 * It provides a simple interface to create a cURLRequest.
 *
 * @tparam T Type of the response body.
 */
template<>
class FactoryRequestWrapper<cURLWrapper> final
{
public:
    /**
     * @brief Create a cURLRequest with the corresponding CurlHandlerTypeEnum for the given enum.
     *
     * @param handlerType Type of the cURL handler. Default is 'SINGLE'.
     * @param shouldRun Flag used to interrupt the cURL handler.
     * @return A shared pointer to a cURLRequest.
     */
    static std::shared_ptr<IRequestImplementator> create(CurlHandlerTypeEnum handlerType = CurlHandlerTypeEnum::SINGLE,
                                                         const std::atomic<bool>& shouldRun = true)
    {
        return std::make_shared<cURLWrapper>(handlerType, shouldRun);
    }
};

#endif // _FACTORY_REQUEST_WRAPPER_HPP
