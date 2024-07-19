/*
 * Wazuh urlRequest TestTool
 * Copyright (C) 2015, Wazuh Inc.
 * July 13, 2022.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _ACTION_HPP
#define _ACTION_HPP
#include "HTTPRequest.hpp"
#include <iostream>

/**
 * @brief Action interface.
 */
class IAction
{
public:
    virtual ~IAction() = default;

    /**
     * @brief Virtual method to execute the action.
     */
    virtual void execute() = 0;
};

/**
 * @brief This class is used to perform a DOWNLOAD action.
 */
class DownloadAction final : public IAction
{
private:
    std::string m_url;
    std::string m_outputFile;
    std::unordered_set<std::string> m_headers;
    SecureCommunication m_secureCommunication;

public:
    /**
     * @brief Constructor for DownloadAction class.
     * @param url URL to download.
     * @param outputFile Output file.
     * @param headers Headers to send in the request.
     * @param secureCommunication Secure communication settings.
     */
    explicit DownloadAction(const std::string& url,
                            const std::string& outputFile,
                            const std::unordered_set<std::string>& headers,
                            const SecureCommunication& secureCommunication)
        : m_url(url)
        , m_outputFile(outputFile)
        , m_headers(headers)
        , m_secureCommunication(secureCommunication)
    {
    }

    /**
     * @brief Executes the action.
     */
    void execute() override
    {
        HTTPRequest::instance().download(
            HttpURL(m_url),
            m_outputFile,
            [](const std::string& msg, const long responseCode)
            {
                std::cerr << msg << ": " << responseCode << std::endl;
                throw std::runtime_error(msg);
            },
            m_headers,
            m_secureCommunication);
    }
};

/**
 * @brief This class is used to perform a GET action.
 */
class GetAction final : public IAction
{
private:
    std::string m_url;
    std::unordered_set<std::string> m_headers;
    SecureCommunication m_secureCommunication;

public:
    /**
     * @brief Constructor of GetAction class.
     * @param url URL to perform the GET request.
     * @param headers Headers to send in the request.
     * @param secureCommunication Secure communication settings.
     */
    explicit GetAction(const std::string& url,
                       const std::unordered_set<std::string>& headers,
                       const SecureCommunication& secureCommunication)
        : m_url(url)
        , m_headers(headers)
        , m_secureCommunication(secureCommunication)
    {
    }

    /**
     * @brief This method is used to perform the GET request.
     */
    void execute() override
    {
        HTTPRequest::instance().get(
            HttpURL(m_url),
            [](const std::string& msg) { std::cout << msg << std::endl; },
            [](const std::string& msg, const long responseCode)
            {
                std::cerr << msg << ": " << responseCode << std::endl;
                throw std::runtime_error(msg);
            },
            "",
            m_headers,
            m_secureCommunication);
    }
};

/**
 * @brief This class is used to perform a POST action.
 */
class PostAction final : public IAction
{
private:
    std::string m_url;
    nlohmann::json m_data;
    std::unordered_set<std::string> m_headers;
    SecureCommunication m_secureCommunication;

public:
    /**
     * @brief Constructor of PostAction class.
     * @param url URL to perform the POST request.
     * @param data Data to send in the POST request.
     * @param headers Headers to send in the request.
     * @param secureCommunication Secure communication settings.
     */
    explicit PostAction(const std::string& url,
                        const nlohmann::json& data,
                        const std::unordered_set<std::string>& headers,
                        const SecureCommunication& secureCommunication)
        : m_url(url)
        , m_data(data)
        , m_headers(headers)
        , m_secureCommunication(secureCommunication)
    {
    }

    /**
     * @brief This method is used to perform the POST request.
     */
    void execute() override
    {
        HTTPRequest::instance().post(
            HttpURL(m_url),
            m_data,
            [](const std::string& msg) { std::cout << msg << std::endl; },
            [](const std::string& msg, const long responseCode)
            {
                std::cerr << msg << ": " << responseCode << std::endl;
                throw std::runtime_error(msg);
            },
            "",
            m_headers,
            m_secureCommunication);
    }
};

/**
 * @brief This class is used to perform a PUT action.
 */
class PutAction final : public IAction
{
private:
    std::string m_url;
    nlohmann::json m_data;
    std::unordered_set<std::string> m_headers;
    SecureCommunication m_secureCommunication;

public:
    /**
     * @brief Constructor of PutAction class.
     * @param url URL to perform the PUT request.
     * @param data Data to send in the PUT request.
     * @param headers Headers to send in the request.
     * @param secureCommunication Secure communication settings.
     */
    explicit PutAction(const std::string& url,
                       const nlohmann::json& data,
                       const std::unordered_set<std::string>& headers,
                       const SecureCommunication& secureCommunication)
        : m_url(url)
        , m_data(data)
        , m_headers(headers)
        , m_secureCommunication(secureCommunication)
    {
    }

    /**
     * @brief This method is used to perform the PUT request.
     */
    void execute() override
    {
        HTTPRequest::instance().put(
            HttpURL(m_url),
            m_data,
            [](const std::string& msg) { std::cout << msg << std::endl; },
            [](const std::string& msg, const long responseCode)
            {
                std::cerr << msg << ": " << responseCode << std::endl;
                throw std::runtime_error(msg);
            },
            "",
            m_headers,
            m_secureCommunication);
    }
};

/**
 * @brief This class is used to perform a PATCH action.
 *
 */
class PatchAction final : public IAction
{
private:
    std::string m_url;
    nlohmann::json m_data;
    std::unordered_set<std::string> m_headers;
    SecureCommunication m_secureCommunication;

public:
    /**
     * @brief Constructor of PatchAction class.
     *
     * @param url URL to perform the PATCH request.
     * @param data Data to send in the PATCH request.
     * @param headers Headers to send in the request.
     * @param secureCommunication Secure communication settings.
     */
    explicit PatchAction(const std::string& url,
                         const nlohmann::json& data,
                         const std::unordered_set<std::string>& headers,
                         const SecureCommunication& secureCommunication)
        : m_url(url)
        , m_data(data)
        , m_headers(headers)
        , m_secureCommunication(secureCommunication)
    {
    }

    /**
     * @brief This method is used to perform the PATCH request.
     *
     */
    void execute() override
    {
        HTTPRequest::instance().patch(
            HttpURL(m_url),
            m_data,
            [](const std::string& msg) { std::cout << msg << std::endl; },
            [](const std::string& msg, const long responseCode)
            {
                std::cerr << msg << ": " << responseCode << std::endl;
                throw std::runtime_error(msg);
            },
            "",
            m_headers,
            m_secureCommunication);
    }
};

/**
 * @brief This class is used to perform a DELETE action.
 */
class DeleteAction final : public IAction
{
private:
    std::string m_url;
    std::unordered_set<std::string> m_headers;
    SecureCommunication m_secureCommunication;

public:
    /**
     * @brief Constructor of DeleteAction class.
     * @param url URL to perform the DELETE request.
     * @param headers Headers to send in the request.
     * @param secureCommunication Secure communication settings.
     */
    explicit DeleteAction(const std::string& url,
                          const std::unordered_set<std::string>& headers,
                          const SecureCommunication& secureCommunication)
        : m_url(url)
        , m_headers(headers)
        , m_secureCommunication(secureCommunication)
    {
    }

    /**
     * @brief This method is used to perform the DELETE request.
     */
    void execute() override
    {
        HTTPRequest::instance().delete_(
            HttpURL(m_url),
            [](const std::string& msg) { std::cout << msg << std::endl; },
            [](const std::string& msg, const long responseCode)
            {
                std::cerr << msg << ": " << responseCode << std::endl;
                throw std::runtime_error(msg);
            },
            "",
            m_headers,
            m_secureCommunication);
    }
};

#endif // _ACTION_HPP
