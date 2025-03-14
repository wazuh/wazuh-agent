#pragma once

#include "commonDefs.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class DBSyncTest : public ::testing::Test
{
protected:
    DBSyncTest() = default;
    virtual ~DBSyncTest() = default;

    void SetUp() override;
    void TearDown() override;
};

struct DummyContext
{
    DBSYNC_HANDLE handle;
    TXN_HANDLE txnContext;
};
