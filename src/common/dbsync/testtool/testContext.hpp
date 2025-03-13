#pragma once

#include "dbsync.h"
#include "dbsync.hpp"

struct TestContext
{
    DBSYNC_HANDLE handle;
    TXN_HANDLE txnContext;
    size_t currentId;
    std::string outputPath;
};
