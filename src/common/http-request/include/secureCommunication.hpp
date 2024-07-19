/*
 * Wazuh shared modules utils
 * Copyright (C) 2015, Wazuh Inc.
 * Oct 30, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef __SECURE_COMMUNICATION_HPP
#define __SECURE_COMMUNICATION_HPP

#include "builder.hpp"
#include <map>
#include <string>

enum class AuthenticationParameter
{
    SSL_CERTIFICATE,
    SSL_KEY,
    CA_ROOT_CERTIFICATE,
    BASIC_AUTH_CREDS
};

/**
 * @brief SecureCommunication class.
 *
 */
class SecureCommunication final : public Utils::Builder<SecureCommunication>
{
private:
    std::map<AuthenticationParameter, std::string> m_parameters;

public:
    /**
     * @brief Set the Client Authentication.
     *
     * @param sslCertificate SSL certificate path.
     */
    SecureCommunication& sslCertificate(const std::string& sslCertificate)
    {
        m_parameters[AuthenticationParameter::SSL_CERTIFICATE] = sslCertificate;

        return (*this);
    }

    /**
     * @brief Set the client key.
     *
     * @param sslKey SSL key path.
     */
    SecureCommunication& sslKey(const std::string& sslKey)
    {
        m_parameters[AuthenticationParameter::SSL_KEY] = sslKey;

        return (*this);
    }

    /**
     * @brief Set the CA Root Certificate.
     *
     * @param caRootCertificate CA certificate path.
     */
    SecureCommunication& caRootCertificate(const std::string& caRootCertificate)
    {
        m_parameters[AuthenticationParameter::CA_ROOT_CERTIFICATE] = caRootCertificate;

        return (*this);
    }

    /**
     * @brief Set the Basic Authentication credentials.
     *
     * @param basicAuthCreds Username and password.
     */
    SecureCommunication& basicAuth(const std::string& basicAuthCreds)
    {
        m_parameters[AuthenticationParameter::BASIC_AUTH_CREDS] = basicAuthCreds;

        return (*this);
    }

    /**
     * @brief Get parameters.
     *
     * @param parameter AuthenticationParameter Parameter to get.
     *
     * @return std::string Parameter value.
     */
    std::string getParameter(const AuthenticationParameter parameter) const
    {
        auto it = m_parameters.find(parameter);
        if (it != m_parameters.end())
        {
            return it->second;
        }
        return {};
    }
};

#endif // __SECURE_COMMUNICATION_HPP
