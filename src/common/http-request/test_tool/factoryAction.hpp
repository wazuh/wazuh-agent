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

#ifndef _FACTORY_ACTION_HPP
#define _FACTORY_ACTION_HPP

#include "actions.hpp"
#include "cmdArgsParser.hpp"
#include <memory>

/**
 * @brief This class is used to build an action.
 */
class FactoryAction final
{
public:
    /**
     * @brief Builds an action.
     * @param args Arguments.
     * @return Shared pointer to the action.
     */
    static std::unique_ptr<IAction> create(const CmdLineArgs& args)
    {
        std::string caRootCertificate {args.cacert()};
        std::string sslCertificate {args.cert()};
        std::string sslKey {args.key()};
        std::string username {args.username()};
        std::string password {args.password()};

        auto secureCommunication = SecureCommunication::builder();
        secureCommunication.basicAuth(username + ":" + password)
            .sslCertificate(sslCertificate)
            .sslKey(sslKey)
            .caRootCertificate(caRootCertificate);

        std::unordered_set<std::string> headers;
        if (args.headers().empty())
        {
            headers = DEFAULT_HEADERS;
        }
        else
        {
            headers.emplace(args.headers());
        }

        if (0 == args.type().compare("download"))
        {
            return std::make_unique<DownloadAction>(args.url(), args.outputFile(), headers, secureCommunication);
        }
        else if (0 == args.type().compare("get"))
        {
            return std::make_unique<GetAction>(args.url(), headers, secureCommunication);
        }
        else if (0 == args.type().compare("post"))
        {
            return std::make_unique<PostAction>(args.url(), args.postArguments(), headers, secureCommunication);
        }
        else if (0 == args.type().compare("put"))
        {
            return std::make_unique<PutAction>(args.url(), args.postArguments(), headers, secureCommunication);
        }
        else if (0 == args.type().compare("patch"))
        {
            return std::make_unique<PatchAction>(args.url(), args.postArguments(), headers, secureCommunication);
        }
        else if (0 == args.type().compare("delete"))
        {
            return std::make_unique<DeleteAction>(args.url(), headers, secureCommunication);
        }
        else
        {
            throw std::runtime_error("Unknown action type");
        }
        return {};
    }
};

#endif // _FACTORY_ACTION_HPP
