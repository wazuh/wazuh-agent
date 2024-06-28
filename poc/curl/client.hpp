#pragma once

#include "command_dispatcher.hpp"
#include "db/rocksdb_wrapper.hpp"
#include "db/sqlite_wrapper.hpp"
#include "event_queue_monitor.hpp"
#include "logger.hpp"
#include "requests.hpp"

#include <memory>
#include <string>
#include <thread>

struct Client
{
    Client()
    {
        Logger::log("CLIENT", "Starting client...");

        ReadCredentials();

        // Login to get new token
        SendLoginRequest(url, uuid, password, token);

        // Start command dispatcher
        commandDispatcher = std::make_unique<CommandDispatcher<RocksDBWrapper>>(
            [this]() { return SendCommandsRequest(this->url, this->uuid, this->password, this->token); });

        // Start queue monitoring
        std::unique_ptr<DBWrapper> p = std::make_unique<SQLiteWrapper>();
        eventQueueMonitor = std::make_unique<EventQueueMonitor>(
            std::move(p),
            [this](const std::string& event)
            { return SendStatelessRequest(this->url, this->uuid, this->password, this->token, event); });
    }

    void ReadCredentials()
    {
        Logger::log("CLIENT", "Reading credentials...");
        url = kURL;
        uuid = kUUID;
        password = kPASSWORD;
        name = kNAME;
        ip = kIP;
    }

    std::string url;
    std::string uuid;
    std::string password;
    std::string token;
    std::string name;
    std::string ip;
    std::unique_ptr<EventQueueMonitor> eventQueueMonitor;
    std::unique_ptr<CommandDispatcher<RocksDBWrapper>> commandDispatcher;
};