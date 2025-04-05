#pragma once

#include <dbsync.hpp>

#include <string>

std::string MockDBSyncDbFilePath;

class MockDBSync
{
public:
    explicit MockDBSync(
        const HostType, const DbEngineType, const std::string& dbFilePath, const std::string&, const DbManagement)
    {
        MockDBSyncDbFilePath = dbFilePath;
    }
};
