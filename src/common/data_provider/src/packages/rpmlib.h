#pragma once

#include "rpmlibWrapper.h"

class RpmLib final : public IRpmLibWrapper
{
public:
    /// @copydoc IRpmLibWrapper::rpmReadConfigFiles
    int rpmReadConfigFiles(const char* file, const char* target) override
    {
        return ::rpmReadConfigFiles(file, target);
    }

    /// @copydoc IRpmLibWrapper::rpmFreeRpmrc
    void rpmFreeRpmrc() override
    {
        ::rpmFreeRpmrc();
    }

    /// @copydoc IRpmLibWrapper::rpmtdNew
    rpmtd rpmtdNew() override
    {
        return ::rpmtdNew();
    }

    /// @copydoc IRpmLibWrapper::rpmtdFree
    void rpmtdFree(rpmtd td) override
    {
        ::rpmtdFree(td);
    }

    /// @copydoc IRpmLibWrapper::rpmtsCreate
    rpmts rpmtsCreate() override
    {
        return ::rpmtsCreate();
    }

    /// @copydoc IRpmLibWrapper::rpmtsOpenDB
    int rpmtsOpenDB(rpmts ts, int dbmode) override
    {
        return ::rpmtsOpenDB(ts, dbmode);
    }

    /// @copydoc IRpmLibWrapper::rpmtsCloseDB
    int rpmtsCloseDB(rpmts ts) override
    {
        return ::rpmtsCloseDB(ts);
    }

    /// @copydoc IRpmLibWrapper::rpmtsFree
    rpmts rpmtsFree(rpmts ts) override
    {
        return ::rpmtsFree(ts);
    }

    /// @copydoc IRpmLibWrapper::headerGet
    int headerGet(Header h, rpmTagVal tag, rpmtd td, headerGetFlags flags) override
    {
        return ::headerGet(h, tag, td, flags);
    }

    /// @copydoc IRpmLibWrapper::rpmtdGetString
    const char* rpmtdGetString(rpmtd td) override
    {
        return ::rpmtdGetString(td);
    }

    /// @copydoc IRpmLibWrapper::rpmtdGetNumber
    uint64_t rpmtdGetNumber(rpmtd td) override
    {
        return ::rpmtdGetNumber(td);
    }

    /// @copydoc IRpmLibWrapper::rpmtsRun
    int rpmtsRun(rpmts ts, rpmps okProbs, rpmprobFilterFlags ignoreSet) override
    {
        return ::rpmtsRun(ts, okProbs, ignoreSet);
    }

    /// @copydoc IRpmLibWrapper::rpmtsInitIterator
    rpmdbMatchIterator
    rpmtsInitIterator(const rpmts ts, rpmDbiTagVal rpmtag, const void* keypointer, size_t keylen) override
    {
        return ::rpmtsInitIterator(ts, rpmtag, keypointer, keylen);
    }

    /// @copydoc IRpmLibWrapper::rpmdbNextIterator
    Header rpmdbNextIterator(rpmdbMatchIterator mi) override
    {
        return ::rpmdbNextIterator(mi);
    }

    /// @copydoc IRpmLibWrapper::rpmdbFreeIterator
    rpmdbMatchIterator rpmdbFreeIterator(rpmdbMatchIterator mi) override
    {
        return ::rpmdbFreeIterator(mi);
    }
};
