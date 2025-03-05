/*
 * Wazuh SYSINFO
 * Copyright (C) 2015, Wazuh Inc.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "packageLinuxParserHelper.h"
#include "sharedDefs.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>

namespace http = boost::beast::http;

void getSnapInfo(std::function<void(nlohmann::json&)> callback)
{
    try
    {
        boost::asio::io_context io_context;
        boost::asio::local::stream_protocol::socket socket(io_context);

        socket.connect("/run/snapd.socket");
        http::request<http::string_body> req(http::verb::get, "/v2/snaps", 11);
        req.set(http::field::host, "localhost");
        req.set(http::field::connection, "close");

        boost::beast::flat_buffer buffer;
        http::write(socket, req);
        http::response<http::string_body> res;
        http::read(socket, buffer, res);

        auto feed = nlohmann::json::parse(res.body(), nullptr, false);
        if (feed.is_discarded() && !feed.contains("result") && feed.at("result").is_array())
        {
            throw std::runtime_error {"Invalid JSON."};
        }

        auto result = feed.at("result");
        for (const auto& entry : result)
        {
            nlohmann::json mapping = PackageLinuxHelper::parseSnap(entry);
            if (!mapping.empty())
            {
                callback(mapping);
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error retrieving packages using snap unix-socket: " << e.what() << std::endl;
    }
}
