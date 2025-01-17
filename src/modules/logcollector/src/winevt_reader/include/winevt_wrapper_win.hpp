#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include <winevt.h>


namespace logcollector::winevt
{

class IWinAPIWrapper
{
public:
    virtual ~IWinAPIWrapper() = default;

    virtual EVT_HANDLE EvtSubscribe(
        HANDLE session,
        HANDLE signalEvent,
        LPCWSTR channelPath,
        LPCWSTR query,
        EVT_HANDLE bookmark,
        PVOID context,
        EVT_SUBSCRIBE_CALLBACK callback,
        DWORD flags) = 0;

    virtual BOOL EvtRender(
        EVT_HANDLE context,
        EVT_HANDLE fragment,
        DWORD flags,
        DWORD bufferSize,
        PVOID buffer,
        DWORD* bufferUsed,
        DWORD* propertyCount) = 0;

    virtual BOOL EvtClose(EVT_HANDLE object) = 0;
};

class DefaultWinAPIWrapper : public IWinAPIWrapper
{
public:
    EVT_HANDLE EvtSubscribe(
        HANDLE session,
        HANDLE signalEvent,
        LPCWSTR channelPath,
        LPCWSTR query,
        EVT_HANDLE bookmark,
        PVOID context,
        EVT_SUBSCRIBE_CALLBACK callback,
        DWORD flags) override
    {
        return ::EvtSubscribe(session, signalEvent, channelPath, query, bookmark, context, callback, flags);
    }

    BOOL EvtRender(
        EVT_HANDLE context,
        EVT_HANDLE fragment,
        DWORD flags,
        DWORD bufferSize,
        PVOID buffer,
        DWORD* bufferUsed,
        DWORD* propertyCount) override
    {
        return ::EvtRender(context, fragment, flags, bufferSize, buffer, bufferUsed, propertyCount);
    }

    BOOL EvtClose(EVT_HANDLE object) override
    {
        return ::EvtClose(object);
    }
};

}
