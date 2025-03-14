#pragma once

#include "commonDefs.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class DBSyncFimIntegrationTest : public ::testing::Test
{
protected:
    DBSyncFimIntegrationTest();
    virtual ~DBSyncFimIntegrationTest();

    void SetUp() override;
    void TearDown() override;
    const DBSYNC_HANDLE m_dbHandle;
};
