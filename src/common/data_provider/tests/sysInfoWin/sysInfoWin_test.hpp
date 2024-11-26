#pragma once

#include "utilsWrapperWin.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SysInfoWinTest : public ::testing::Test
{
protected:
    SysInfoWinTest() = default;
    virtual ~SysInfoWinTest() = default;

    void SetUp() override;
    void TearDown() override;
};

class MockComHelper : public IComHelper
{
public:
    MOCK_METHOD(HRESULT, CreateWmiLocator, (IWbemLocator * &pLoc), (override));
    MOCK_METHOD(HRESULT, ConnectToWmiServer, (IWbemLocator * pLoc, IWbemServices*& pSvc), (override));
    MOCK_METHOD(HRESULT, SetProxyBlanket, (IWbemServices * pSvc), (override));
    MOCK_METHOD(HRESULT, ExecuteWmiQuery, (IWbemServices * pSvc, IEnumWbemClassObject*& pEnumerator), (override));
    MOCK_METHOD(HRESULT, CreateUpdateSearcher, (IUpdateSearcher * &pUpdateSearcher), (override));
    MOCK_METHOD(HRESULT, GetTotalHistoryCount, (IUpdateSearcher * pUpdateSearcher, LONG& count), (override));
    MOCK_METHOD(HRESULT,
                QueryHistory,
                (IUpdateSearcher * pUpdateSearcher, IUpdateHistoryEntryCollection*& pHistory, LONG& count),
                (override));
    MOCK_METHOD(HRESULT, GetCount, (IUpdateHistoryEntryCollection * pHistory, LONG& count), (override));
    MOCK_METHOD(HRESULT,
                GetItem,
                (IUpdateHistoryEntryCollection * pHistory, LONG index, IUpdateHistoryEntry** pEntry),
                (override));
    MOCK_METHOD(HRESULT, GetTitle, (IUpdateHistoryEntry * pEntry, BSTR& title), (override));
};
