/*
 * Wazuh cURLHandlerCache unit tests
 * Copyright (C) 2015, Wazuh Inc.
 * December 28, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */
#include "curlHandlerCache_test.hpp"
#include "curlHandlerCache.hpp"
#include "curlMultiHandler.hpp"
#include "curlSingleHandler.hpp"
#include "curlWrapper.hpp"
#include <memory>

/*
 * @brief Test the creation of the Single handler.
 */
TEST_F(cURLHandlerCacheTest, SingleHandlerCreation)
{
    // Create the cURL handler and check that it is a Single handler.
    EXPECT_TRUE(std::dynamic_pointer_cast<cURLSingleHandler>(
        cURLHandlerCache::instance().getCurlHandler(CurlHandlerTypeEnum::SINGLE)));
}

/*
 * @brief Test the creation of the Multi handler.
 */
TEST_F(cURLHandlerCacheTest, MultiHandlerCreation)
{
    // Create the cURL handler and check that it is a Multi handler.
    EXPECT_TRUE(std::dynamic_pointer_cast<cURLMultiHandler>(
        cURLHandlerCache::instance().getCurlHandler(CurlHandlerTypeEnum::MULTI)));
}

/**
 * @brief This test checks the behavior of the single-handler in multiple threads
 */
TEST_F(cURLHandlerCacheTest, SingleHandlerInMultipleThreads)
{
    std::vector<std::thread> threads;
    for (int i = 0; i < QUEUE_MAX_SIZE * 2; ++i)
    {
        threads.emplace_back(
            []() { EXPECT_NO_THROW(cURLHandlerCache::instance().getCurlHandler(CurlHandlerTypeEnum::SINGLE)); });

        EXPECT_LE(cURLHandlerCache::instance().size(), QUEUE_MAX_SIZE);
    }

    for (auto& thread : threads)
    {
        EXPECT_NO_THROW(thread.join());
    }
}

/**
 * @brief This test checks the behavior of the multi-handler in multiple threads
 */
TEST_F(cURLHandlerCacheTest, MultiHandlerInMultipleThreads)
{
    std::vector<std::thread> threads;
    for (int i = 0; i < QUEUE_MAX_SIZE * 2; ++i)
    {
        threads.emplace_back(
            []() { EXPECT_NO_THROW(cURLHandlerCache::instance().getCurlHandler(CurlHandlerTypeEnum::MULTI)); });

        EXPECT_LE(cURLHandlerCache::instance().size(), QUEUE_MAX_SIZE);
    }

    for (auto& thread : threads)
    {
        EXPECT_NO_THROW(thread.join());
    }
}

/**
 * @brief This test checks the instantiation of the single-handler and multi-handler in the same thread.
 */
TEST_F(cURLHandlerCacheTest, SingleHandlerAndMultiHandlerInTheSameThread)
{
    std::thread thread1(
        []()
        {
            // Create the single-handler
            EXPECT_NO_THROW(cURLHandlerCache::instance().getCurlHandler(CurlHandlerTypeEnum::SINGLE));

            // Create the multi-handler
            EXPECT_NO_THROW(cURLHandlerCache::instance().getCurlHandler(CurlHandlerTypeEnum::MULTI));

            // Check the elements in the cache. It should be two since they are two different types of handlers
            EXPECT_LE(cURLHandlerCache::instance().size(), 2);
        });

    EXPECT_NO_THROW(thread1.join());
}

/**
 * @brief This test checks the instantiation of two the single-handler in the same thread.
 */
TEST_F(cURLHandlerCacheTest, TwoSingleHandlerInTheSameThread)
{
    std::thread thread1(
        []()
        {
            // Create the first single-handler
            EXPECT_NO_THROW(cURLHandlerCache::instance().getCurlHandler(CurlHandlerTypeEnum::SINGLE));

            // Create the second single-handler
            EXPECT_NO_THROW(cURLHandlerCache::instance().getCurlHandler(CurlHandlerTypeEnum::SINGLE));

            // Check the elements in the cache. It should be one since the handler was created in the same thread
            EXPECT_LE(cURLHandlerCache::instance().size(), 1);
        });

    EXPECT_NO_THROW(thread1.join());
}

/**
 * @brief This test checks the instantiation of two the multi-handler in the same thread.
 */
TEST_F(cURLHandlerCacheTest, TwoMultiHandlerInTheSameThread)
{
    std::thread thread1(
        []()
        {
            // Create the first multi-handler
            EXPECT_NO_THROW(cURLHandlerCache::instance().getCurlHandler(CurlHandlerTypeEnum::MULTI));

            // Create the second multi-handler
            EXPECT_NO_THROW(cURLHandlerCache::instance().getCurlHandler(CurlHandlerTypeEnum::MULTI));

            // Check the elements in the cache. It should be one since the handler was created in the same thread
            EXPECT_LE(cURLHandlerCache::instance().size(), 1);
        });

    EXPECT_NO_THROW(thread1.join());
}
