/*
 * Wazuh urlRequest test component
 * Copyright (C) 2015, Wazuh Inc.
 * July 18, 2022.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "component_test.hpp"
#include "curlHandlerCache.hpp"
#include "curlWrapper.hpp"
#include "factoryRequestImplemetator.hpp"
#include "json.hpp"
#include "urlRequest.hpp"
#include <map>
#include <string>

/**
 * @brief Test the get request.
 */
TEST_F(ComponentTestInterface, GetHelloWorld)
{
    HTTPRequest::instance().get(HttpURL("http://localhost:44441/"),
                                [&](const std::string& result)
                                {
                                    EXPECT_EQ(result, "Hello World!");
                                    m_callbackComplete = true;
                                });

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the get request with redirection.
 */
TEST_F(ComponentTestInterface, GetHelloWorldRedirection)
{
    HTTPRequest::instance().get(HttpURL("http://localhost:44441/redirect"),
                                [&](const std::string& result)
                                {
                                    EXPECT_EQ(result, "Hello World!");
                                    m_callbackComplete = true;
                                });

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the post request.
 */
TEST_F(ComponentTestInterface, PostHelloWorld)
{
    HTTPRequest::instance().post(HttpURL("http://localhost:44441/"),
                                 R"({"hello":"world"})"_json,
                                 [&](const std::string& result)
                                 {
                                     EXPECT_EQ(result, R"({"hello":"world"})");
                                     m_callbackComplete = true;
                                 });

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the update request.
 */
TEST_F(ComponentTestInterface, PutHelloWorld)
{
    HTTPRequest::instance().put(HttpURL("http://localhost:44441/"),
                                R"({"hello":"world"})"_json,
                                [&](const std::string& result)
                                {
                                    EXPECT_EQ(result, R"({"hello":"world"})");
                                    m_callbackComplete = true;
                                });

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the delete request.
 */
TEST_F(ComponentTestInterface, DeleteRandomID)
{
    auto random {std::to_string(std::rand())};

    HTTPRequest::instance().delete_(HttpURL("http://localhost:44441/" + random),
                                    [&](const std::string& result)
                                    {
                                        EXPECT_EQ(result, random);
                                        m_callbackComplete = true;
                                    });

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the download request.
 */
TEST_F(ComponentTestInterface, DownloadFile)
{
    HTTPRequest::instance().download(HttpURL("http://localhost:44441/"), "./test.txt", [](auto, auto) {});

    std::ifstream file("./test.txt");
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "Hello World!");
}

/**
 * @brief Test the download request with empty URL.
 */
TEST_F(ComponentTestInterface, DownloadFileEmptyURL)
{
    HTTPRequest::instance().download(HttpURL(""),
                                     "./test.txt",
                                     [&](const std::string& result, const long responseCode)
                                     {
                                         EXPECT_EQ(result, "URL using bad/illegal format or missing URL");
                                         EXPECT_EQ(responseCode, -1);

                                         m_callbackComplete = true;
                                     });

    std::ifstream file("./test.txt");
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "");
}

/**
 * @brief Test the download request with a invalid URL.
 */
TEST_F(ComponentTestInterface, DownloadFileError)
{
    HTTPRequest::instance().download(HttpURL("http://localhost:44441/invalid_file"),
                                     "./test.txt",
                                     [&](const std::string& result, const long responseCode)
                                     {
                                         EXPECT_EQ(result, "HTTP response code said error");
                                         EXPECT_EQ(responseCode, 404);

                                         m_callbackComplete = true;
                                     });

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the custom download request using the single handler.
 */
TEST_F(ComponentTestInterface, DownloadFileUsingTheSingleHandler)
{
    HTTPRequest::instance().download(
        HttpURL("http://localhost:44441/"), "./test.txt", [](auto, auto) {}, {}, {}, {}, CurlHandlerTypeEnum::SINGLE);

    std::ifstream file("./test.txt");
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "Hello World!");
}

/**
 * @brief Test the custom download request using the single handler with empty URL.
 */
TEST_F(ComponentTestInterface, DownloadFileEmptyURLUsingTheSingleHandler)
{
    HTTPRequest::instance().download(
        HttpURL(""),
        "./test.txt",
        [&](const std::string& result, const long responseCode)
        {
            EXPECT_EQ(result, "URL using bad/illegal format or missing URL");
            EXPECT_EQ(responseCode, -1);

            m_callbackComplete = true;
        },
        {},
        {},
        {},
        CurlHandlerTypeEnum::SINGLE);

    std::ifstream file("./test.txt");
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "");
}

/**
 * @brief Test the custom download request using the single handler with a invalid URL.
 */
TEST_F(ComponentTestInterface, DownloadFileErrorUsingTheSingleHandler)
{
    HTTPRequest::instance().download(
        HttpURL("http://localhost:44441/invalid_file"),
        "./test.txt",
        [&](const std::string& result, const long responseCode)
        {
            EXPECT_EQ(result, "HTTP response code said error");
            EXPECT_EQ(responseCode, 404);

            m_callbackComplete = true;
        },
        {},
        {},
        {},
        CurlHandlerTypeEnum::SINGLE);

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the custom download request using the multi handler.
 */
TEST_F(ComponentTestInterface, DownloadFileUsingTheMultiHandler)
{
    std::atomic<bool> shouldRun {true};

    HTTPRequest::instance().download(
        HttpURL("http://localhost:44441/"),
        "./test.txt",
        [](auto, auto) {},
        {},
        {},
        {},
        CurlHandlerTypeEnum::MULTI,
        shouldRun);

    std::ifstream file("./test.txt");
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "Hello World!");
}

/**
 * @brief Test the custom download request using the multi handler and interrupt the handler.
 */
TEST_F(ComponentTestInterface, InterruptMultiHandler)
{
    std::atomic<bool> shouldRun {false};

    HTTPRequest::instance().download(
        HttpURL("http://localhost:44441/"),
        "./test.txt",
        [](auto, auto) {},
        {},
        {},
        {},
        CurlHandlerTypeEnum::MULTI,
        shouldRun);

    std::ifstream file("./test.txt");
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "");
}

/**
 * @brief Test two instances of the custom download request using the multi handler and interrupt the handler.
 *
 */
TEST_F(ComponentTestInterface, InterruptDownload)
{
    std::atomic<bool> shouldRun {true};

    auto sleepFirstHandler {std::to_string(10)};
    auto sleepSecondHandler {std::to_string(40)};
    auto intervalToInterruptTheHandler {20};

    std::thread thread1(
        [&shouldRun, &sleepFirstHandler]()
        {
            HTTPRequest::instance().download(
                HttpURL("http://localhost:44441/sleep/" + sleepFirstHandler),
                "./test1.txt",
                [](auto, auto) {},
                {},
                {},
                {},
                CurlHandlerTypeEnum::MULTI,
                shouldRun);
        });

    std::thread thread2(
        [&shouldRun, &sleepSecondHandler]()
        {
            HTTPRequest::instance().download(
                HttpURL("http://localhost:44441/sleep/" + sleepSecondHandler),
                "./test2.txt",
                [](auto, auto) {},
                {},
                {},
                {},
                CurlHandlerTypeEnum::MULTI,
                shouldRun);
        });

    // Sleep interval to interrupt the second handler.
    std::this_thread::sleep_for(std::chrono::milliseconds(intervalToInterruptTheHandler));
    shouldRun.store(false);

    thread1.join();
    thread2.join();

    std::ifstream file1("./test1.txt");
    std::string line1;
    std::getline(file1, line1);
    EXPECT_EQ(line1, "Hello World!");

    std::ifstream file2("./test2.txt");
    std::string line2;
    std::getline(file2, line2);
    // As the second thread was interrupted, there is no response from the endpoint 'sleep'
    EXPECT_EQ(line2, "");
}

/**
 * @brief Test the custom download request using the multi handler with empty URL.
 */
TEST_F(ComponentTestInterface, DownloadFileEmptyURLUsingTheMultiHandler)
{
    std::atomic<bool> shouldRun {true};

    HTTPRequest::instance().download(
        HttpURL(""),
        "./test.txt",
        [&](const std::string& result, const long responseCode)
        {
            EXPECT_EQ(result, "cURLMultiHandler::execute() failed: curl_multi_add_handle");
            EXPECT_EQ(responseCode, -1);

            m_callbackComplete = true;
        },
        {},
        {},
        {},
        CurlHandlerTypeEnum::MULTI,
        shouldRun);

    std::ifstream file("./test.txt");
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "");
}

/**
 * @brief Test the custom download request using the multi handler with a invalid URL.
 */
TEST_F(ComponentTestInterface, DownloadFileErrorUsingTheMultiHandler)
{
    std::atomic<bool> shouldRun {true};

    HTTPRequest::instance().download(
        HttpURL("http://localhost:44441/invalid_file"),
        "./test.txt",
        [&](const std::string& result, const long responseCode)
        {
            EXPECT_EQ(result, "cURLMultiHandler::execute() failed: curl_multi_add_handle");
            EXPECT_EQ(responseCode, -1);

            m_callbackComplete = true;
        },
        {},
        {},
        {},
        CurlHandlerTypeEnum::MULTI,
        shouldRun);

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the get request and check the file content.
 */
TEST_F(ComponentTestInterface, GetHelloWorldFile)
{
    HTTPRequest::instance().get(
        HttpURL("http://localhost:44441/"),
        [&](const std::string& result) { std::cout << result << std::endl; },
        [](auto, auto) {},
        "./testGetHelloWorld.txt");

    std::ifstream file("./testGetHelloWorld.txt");
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "Hello World!");
}

/**
 * @brief Test the get request with empty URL.
 */
TEST_F(ComponentTestInterface, GetHelloWorldFileEmptyURL)
{
    HTTPRequest::instance().get(
        HttpURL(""),
        [&](const std::string& result) { std::cout << result << std::endl; },
        [&](const std::string& result, const long responseCode)
        {
            EXPECT_EQ(result, "URL using bad/illegal format or missing URL");
            EXPECT_EQ(responseCode, -1);

            m_callbackComplete = true;
        },
        "./testGetHelloWorld.txt");

    std::ifstream file("./testGetHelloWorld.txt");
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "");
}

/**
 * @brief Test the post request and check the file content.
 */
TEST_F(ComponentTestInterface, PostHelloWorldFile)
{
    HTTPRequest::instance().post(
        HttpURL("http://localhost:44441/"),
        R"({"hello":"world"})"_json,
        [&](const std::string& result) { std::cout << result << std::endl; },
        [](auto, auto) {},
        "./testPostHelloWorld.txt");

    std::ifstream file("./testPostHelloWorld.txt");
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, R"({"hello":"world"})");
}

/**
 * @brief Test the post request with empty URL.
 */
TEST_F(ComponentTestInterface, PostHelloWorldFileEmptyURL)
{
    HTTPRequest::instance().post(
        HttpURL(""),
        R"({"hello":"world"})"_json,
        [&](const std::string& result) { std::cout << result << std::endl; },
        [&](const std::string& result, const long responseCode)
        {
            EXPECT_EQ(result, "URL using bad/illegal format or missing URL");
            EXPECT_EQ(responseCode, -1);

            m_callbackComplete = true;
        },
        "./testPostHelloWorld.txt");

    std::ifstream file("./testPostHelloWorld.txt");
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "");
}

/**
 * @brief Test the update request and check the file content.
 */
TEST_F(ComponentTestInterface, PutHelloWorldFile)
{
    HTTPRequest::instance().put(
        HttpURL("http://localhost:44441/"),
        R"({"hello":"world"})"_json,
        [&](const std::string& result) { std::cout << result << std::endl; },
        [](auto, auto) {},
        "./testPutHelloWorld.txt");

    std::ifstream file("./testPutHelloWorld.txt");
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, R"({"hello":"world"})");
}

/**
 * @brief Test the update request and check the file content.
 */
TEST_F(ComponentTestInterface, PutHelloWorldFileEmptyURL)
{
    HTTPRequest::instance().put(
        HttpURL(""),
        R"({"hello":"world"})"_json,
        [&](const std::string& result) { std::cout << result << std::endl; },
        [&](const std::string& result, const long responseCode)
        {
            EXPECT_EQ(result, "URL using bad/illegal format or missing URL");
            EXPECT_EQ(responseCode, -1);

            m_callbackComplete = true;
        },
        "./testPutHelloWorld.txt");

    std::ifstream file("./testPutHelloWorld.txt");
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "");
}

/**
 * @brief Test the delete request and check the file content.
 */
TEST_F(ComponentTestInterface, DeleteRandomIDFile)
{
    auto random {std::to_string(std::rand())};

    HTTPRequest::instance().delete_(
        HttpURL("http://localhost:44441/" + random),
        [&](const std::string& result) { std::cout << result << std::endl; },
        [](auto, auto) {},
        "./testDeleteRandomID.txt");

    std::ifstream file("./testDeleteRandomID.txt");
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, random);
}

/**
 * @brief Test the delete request with empty URL.
 */
TEST_F(ComponentTestInterface, DeleteRandomIDFileEmptyURL)
{
    auto random {std::to_string(std::rand())};

    HTTPRequest::instance().delete_(
        HttpURL(""),
        [&](const std::string& result) { std::cout << result << std::endl; },
        [&](const std::string& result, const long responseCode)
        {
            EXPECT_EQ(result, "URL using bad/illegal format or missing URL");
            EXPECT_EQ(responseCode, -1);

            m_callbackComplete = true;
        },
        "./testDeleteRandomID.txt");

    std::ifstream file("./testDeleteRandomID.txt");
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "");
}

using wrapperType = cURLWrapper;

/**
 * @brief Test the download request with a empty URL.
 */
TEST_F(ComponentTestInternalParameters, DownloadFileEmptyInvalidUrl)
{
    try
    {
        GetRequest::builder(FactoryRequestWrapper<wrapperType>::create()).url("").outputFile("test.txt").execute();
    }
    catch (const std::exception& ex)
    {
        EXPECT_EQ(std::string(ex.what()), "URL using bad/illegal format or missing URL");
        m_callbackComplete = true;
    }
    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the download request with a invalid URL.
 */
TEST_F(ComponentTestInternalParameters, DownloadFileEmptyInvalidUrl2)
{
    try
    {
        GetRequest::builder(FactoryRequestWrapper<wrapperType>::create())
            .url("http://")
            .outputFile("test.txt")
            .execute();
    }
    catch (const std::exception& ex)
    {
        EXPECT_EQ(std::string(ex.what()), "URL using bad/illegal format or missing URL");
        m_callbackComplete = true;
    }
    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the get request with a invalid file.
 */
TEST_F(ComponentTestInternalParameters, GetError)
{
    try
    {
        GetRequest::builder(FactoryRequestWrapper<wrapperType>::create())
            .url("http://localhost:44441/invalid_file")
            .execute();
    }
    catch (const std::exception& ex)
    {
        EXPECT_EQ(std::string(ex.what()), "HTTP response code said error");
        m_callbackComplete = true;
    }
    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the post request with a invalid file.
 */
TEST_F(ComponentTestInternalParameters, PostError)
{
    try
    {
        PostRequest::builder(FactoryRequestWrapper<wrapperType>::create())
            .url("http://localhost:44441/invalid_file")
            .postData(R"({"hello":"world"})")
            .execute();
    }
    catch (const std::exception& ex)
    {
        EXPECT_EQ(std::string(ex.what()), "HTTP response code said error");
        m_callbackComplete = true;
    }
    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the put request with a invalid file.
 */
TEST_F(ComponentTestInternalParameters, PutError)
{
    try
    {
        PutRequest::builder(FactoryRequestWrapper<wrapperType>::create())
            .url("http://localhost:44441/invalid_file")
            .postData(R"({"hello":"world"})")
            .execute();
    }
    catch (const std::exception& ex)
    {
        EXPECT_EQ(std::string(ex.what()), "HTTP response code said error");
        m_callbackComplete = true;
    }
    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the delete request with a invalid file.
 */
TEST_F(ComponentTestInternalParameters, DeleteError)
{
    try
    {
        DeleteRequest::builder(FactoryRequestWrapper<wrapperType>::create())
            .url("http://localhost:44441/invalid_file")
            .execute();
    }
    catch (const std::exception& ex)
    {
        EXPECT_EQ(std::string(ex.what()), "HTTP response code said error");
        m_callbackComplete = true;
    }
    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the get request with no url.
 */
TEST_F(ComponentTestInternalParameters, ExecuteGetNoUrl)
{
    try
    {
        GetRequest::builder(FactoryRequestWrapper<wrapperType>::create()).execute();
    }
    catch (const std::exception& ex)
    {
        EXPECT_EQ(std::string(ex.what()), "URL using bad/illegal format or missing URL");
        m_callbackComplete = true;
    }
    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the post request with no url.
 */
TEST_F(ComponentTestInternalParameters, ExecutePostNoUrl)
{
    try
    {
        PostRequest::builder(FactoryRequestWrapper<wrapperType>::create()).execute();
    }
    catch (const std::exception& ex)
    {
        EXPECT_EQ(std::string(ex.what()), "URL using bad/illegal format or missing URL");
        m_callbackComplete = true;
    }
    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the put request with no url.
 */
TEST_F(ComponentTestInternalParameters, ExecutePutNoUrl)
{
    try
    {
        PutRequest::builder(FactoryRequestWrapper<wrapperType>::create()).execute();
    }
    catch (const std::exception& ex)
    {
        EXPECT_EQ(std::string(ex.what()), "URL using bad/illegal format or missing URL");
        m_callbackComplete = true;
    }
    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the delete request with no url.
 */
TEST_F(ComponentTestInternalParameters, ExecuteDeleteNoUrl)
{
    try
    {
        DeleteRequest::builder(FactoryRequestWrapper<wrapperType>::create()).execute();
    }
    catch (const std::exception& ex)
    {
        EXPECT_EQ(std::string(ex.what()), "URL using bad/illegal format or missing URL");
        m_callbackComplete = true;
    }
    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief This test checks the behavior of multiple threads.
 * This test create multiple threads that exceed the size of the queue where each thread will create a cURLWrapper
 * object.
 */
TEST_F(ComponentTestInternalParameters, MultipleThreads)
{
    std::vector<std::thread> threads;
    for (int i = 0; i < QUEUE_MAX_SIZE * 2; ++i)
    {
        threads.emplace_back(
            []()
            {
                EXPECT_NO_THROW(GetRequest::builder(FactoryRequestWrapper<wrapperType>::create())
                                    .url("http://localhost:44441/")
                                    .execute());
            });

        EXPECT_LE(cURLHandlerCache::instance().size(), QUEUE_MAX_SIZE);
    }

    for (auto& thread : threads)
    {
        EXPECT_NO_THROW(thread.join());
    }
}

/**
 * @brief This test checks the behavior of multiple threads for multi handler.
 * This test create multiple threads that exceed the size of the queue where each thread will create a cURLWrapper
 * object.
 */
TEST_F(ComponentTestInternalParameters, MultipleThreadsWithMultHandlers)
{
    std::vector<std::thread> threads;
    std::atomic<bool> shouldRun {true};

    for (int i = 0; i < QUEUE_MAX_SIZE * 2; ++i)
    {
        threads.emplace_back(
            [&shouldRun]()
            {
                EXPECT_NO_THROW(GetRequest::builder(
                                    FactoryRequestWrapper<wrapperType>::create(CurlHandlerTypeEnum::MULTI, shouldRun))
                                    .url("http://localhost:44441/")
                                    .execute());
            });

        EXPECT_LE(cURLHandlerCache::instance().size(), QUEUE_MAX_SIZE);
    }
    for (auto& thread : threads)
    {
        EXPECT_NO_THROW(thread.join());
    }
}

/**
 * @brief Test the GET request appending a custom HTTP header. The header is expected to be on the server response.
 *
 */
TEST_F(ComponentTestInterface, GetWithCustomHeader)
{
    const std::string headerKey {"Custom-Key"};
    const std::string headerValue {"Custom-Value"};

    HTTPRequest::instance().get(
        HttpURL("http://localhost:44441/check-headers"),
        [&](const std::string& result)
        {
            ASSERT_EQ(nlohmann::json::parse(result).at(headerKey), headerValue);
            m_callbackComplete = true;
        },
        [](auto, auto) {},
        "",
        {headerKey + ": " + headerValue});

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the GET request without custom HTTP headers. The expected headers are the default ones.
 *
 */
TEST_F(ComponentTestInterface, GetWithDefaultHeaders)
{
    HTTPRequest::instance().get(
        HttpURL("http://localhost:44441/check-headers"),
        [&](const std::string& result)
        {
            const std::map<std::string, std::string> defaultHeaders = {
                {"Content-Type", "application/json"}, {"Accept", "application/json"}, {"Accept-Charset", "utf-8"}};
            const auto response = nlohmann::json::parse(result);

            ASSERT_FALSE(response.empty());
            for (const auto& [headerKey, headerValue] : defaultHeaders)
            {
                ASSERT_EQ(response.at(headerKey), headerValue);
            }

            m_callbackComplete = true;
        });

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the POST request appending two custom HTTP headers. The headers are expected to be on the server
 * response.
 *
 */
TEST_F(ComponentTestInterface, PostWithCustomHeaders)
{
    const std::string headerKeyA {"Custom-Key-A"};
    const std::string headerValueA {"Custom-Value-A"};
    const std::string headerKeyB {"Custom-Key-B"};
    const std::string headerValueB {"Custom-Value-B"};

    HTTPRequest::instance().post(
        HttpURL("http://localhost:44441/check-headers"),
        std::string(),
        [&](const std::string& result)
        {
            const auto response = nlohmann::json::parse(result);

            ASSERT_EQ(response.at(headerKeyA), headerValueA);
            ASSERT_EQ(response.at(headerKeyB), headerValueB);
            m_callbackComplete = true;
        },
        [](auto, auto) {},
        "",
        {headerKeyA + ":" + headerValueA, headerKeyB + ":" + headerValueB});

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the PUT request appending two equal custom HTTP headers. Because headers are inserted in a set (as unique
 * values), just one header is expected to be on the server response.
 *
 */
TEST_F(ComponentTestInterface, PutWithCustomHeaders)
{
    const std::string headerKey {"Custom-Key"};
    const std::string headerValue {"Custom-Value"};

    HTTPRequest::instance().put(
        HttpURL("http://localhost:44441/check-headers"),
        std::string(),
        [&](const std::string& result)
        {
            ASSERT_EQ(nlohmann::json::parse(result).at(headerKey), headerValue);
            m_callbackComplete = true;
        },
        [](auto, auto) {},
        "",
        {headerKey + ":" + headerValue, headerKey + ":" + headerValue});

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the basic functionality of a PATCH request.
 *
 */
TEST_F(ComponentTestInterface, PatchSimpleFunctionality)
{
    const auto postData = R"({"hello":"world"})"_json;

    auto expectedResponse = R"(
        {
            "query": "patch"
        }
    )"_json;
    expectedResponse["payload"] = postData;

    HTTPRequest::instance().patch(HttpURL("http://localhost:44441/"),
                                  postData,
                                  [&](const std::string& response)
                                  {
                                      EXPECT_EQ(nlohmann::json::parse(response), expectedResponse);
                                      m_callbackComplete = true;
                                  });

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the DOWNLOAD request setting a custom user-agent.
 */
TEST_F(ComponentTestInterface, DownloadWithCustomUserAgent)
{
    const std::string userAgent {"Custom-User-Agent"};

    HTTPRequest::instance().download(
        HttpURL("http://localhost:44441/"), "./test.txt", [](auto, auto) {}, DEFAULT_HEADERS, {}, userAgent);

    std::ifstream file("./test.txt");
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "Hello World!");
}

/**
 * @brief Test the POST request setting a custom user-agent.
 */
TEST_F(ComponentTestInterface, PostWithCustomUserAgent)
{
    const std::string headerKey {"User-Agent"};
    const std::string userAgentValue {"Custom-User-Agent"};

    HTTPRequest::instance().post(
        HttpURL("http://localhost:44441/check-headers"),
        R"({"hello":"world"})"_json,
        [&](const std::string& result)
        {
            ASSERT_EQ(nlohmann::json::parse(result).at(headerKey), userAgentValue);
            m_callbackComplete = true;
        },
        [](auto, auto) {},
        "",
        {},
        {},
        userAgentValue);

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the GET request setting a custom user-agent.
 *
 */
TEST_F(ComponentTestInterface, GetWithCustomUserAgent)
{
    const std::string headerKey {"User-Agent"};
    const std::string userAgentValue {"Custom-User-Agent"};

    HTTPRequest::instance().get(
        HttpURL("http://localhost:44441/check-headers"),
        [&](const std::string& result)
        {
            ASSERT_EQ(nlohmann::json::parse(result).at(headerKey), userAgentValue);
            m_callbackComplete = true;
        },
        [](auto, auto) {},
        "",
        {},
        {},
        userAgentValue);

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the PUT request setting a custom user-agent.
 *
 */
TEST_F(ComponentTestInterface, PutWithCustomUserAgent)
{
    const std::string headerKey {"User-Agent"};
    const std::string userAgentValue {"Custom-User-Agent"};

    HTTPRequest::instance().put(
        HttpURL("http://localhost:44441/check-headers"),
        R"({"hello":"world"})"_json,
        [&](const std::string& result)
        {
            ASSERT_EQ(nlohmann::json::parse(result).at(headerKey), userAgentValue);
            m_callbackComplete = true;
        },
        [](auto, auto) {},
        "",
        {},
        {},
        userAgentValue);

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the PATCH request setting a custom user-agent.
 *
 */
TEST_F(ComponentTestInterface, PatchWithCustomUserAgent)
{
    const std::string headerKey {"User-Agent"};
    const std::string userAgentValue {"Custom-User-Agent"};

    HTTPRequest::instance().patch(
        HttpURL("http://localhost:44441/check-headers"),
        R"({"hello":"world"})"_json,
        [&](const std::string& result)
        {
            ASSERT_EQ(nlohmann::json::parse(result).at(headerKey), userAgentValue);
            m_callbackComplete = true;
        },
        [](auto, auto) {},
        "",
        {},
        {},
        userAgentValue);

    EXPECT_TRUE(m_callbackComplete);
}

/**
 * @brief Test the DELETE request setting a custom user-agent.
 *
 */
TEST_F(ComponentTestInterface, DeleteWithCustomUserAgent)
{
    const std::string headerKey {"User-Agent"};
    const std::string userAgentValue {"Custom-User-Agent"};

    HTTPRequest::instance().delete_(
        HttpURL("http://localhost:44441/check-headers"),
        [&](const std::string& result)
        {
            ASSERT_EQ(nlohmann::json::parse(result).at(headerKey), userAgentValue);
            m_callbackComplete = true;
        },
        [](auto, auto) {},
        "",
        {},
        {},
        userAgentValue);

    EXPECT_TRUE(m_callbackComplete);
}
