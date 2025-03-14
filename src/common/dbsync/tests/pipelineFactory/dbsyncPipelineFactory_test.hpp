#pragma once

#include "dbsyncPipelineFactory.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class DBSyncPipelineFactoryTest : public ::testing::Test
{
protected:
    DBSyncPipelineFactoryTest()
        : m_pipelineFactory {DbSync::PipelineFactory::instance()}
        , m_dbHandle {nullptr}
    {
    }

    virtual ~DBSyncPipelineFactoryTest() = default;

    void SetUp() override;
    void TearDown() override;
    DbSync::PipelineFactory& m_pipelineFactory;
    DBSYNC_HANDLE m_dbHandle;
};
