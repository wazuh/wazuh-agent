#pragma once

#include <rpm/header.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>

/// @brief Wrapper for rpmlib
class IRpmLibWrapper
{
public:
    /// @brief Default destructor
    virtual ~IRpmLibWrapper() = default;

    /// @brief Reads rpm config files
    /// @param file Path to rpm config file
    /// @param target Target rpm config file
    /// @return 0 on success
    virtual int rpmReadConfigFiles(const char* file, const char* target) = 0;

    /// @brief Frees rpm config files
    virtual void rpmFreeRpmrc() = 0;

    /// @brief Creates a new rpmtd
    /// @return rpmtd
    virtual rpmtd rpmtdNew() = 0;

    /// @brief Frees a rpmtd
    /// @param td rpmtd
    virtual void rpmtdFree(rpmtd td) = 0;

    /// @brief Creates a new rpmts
    /// @return rpmts
    virtual rpmts rpmtsCreate() = 0;

    /// @brief Opens a database
    /// @param ts rpmts
    /// @param dbmode database mode
    /// @return 0 on success
    virtual int rpmtsOpenDB(rpmts ts, int dbmode) = 0;

    /// @brief Closes a database
    /// @param ts rpmts
    /// @return 0 on success
    virtual int rpmtsCloseDB(rpmts ts) = 0;

    /// @brief Frees a rpmts
    /// @param ts rpmts
    /// @return rpmts
    virtual rpmts rpmtsFree(rpmts ts) = 0;

    /// @brief Gets a header
    /// @param h Header
    /// @param tag Tag
    /// @param td rpmtd
    /// @param flags Flags
    /// @return 0 on success
    virtual int headerGet(Header h, rpmTagVal tag, rpmtd td, headerGetFlags flags) = 0;

    /// @brief Gets a string
    /// @param td rpmtd
    /// @return string
    virtual const char* rpmtdGetString(rpmtd td) = 0;

    /// @brief Gets a number
    /// @param td rpmtd
    /// @return number
    virtual uint64_t rpmtdGetNumber(rpmtd td) = 0;

    /// @brief Runs rpmts
    /// @param ts rpmts
    /// @param okProbs rpmps
    /// @param ignoreSet rpmprobFilterFlags
    /// @return 0 on success
    virtual int rpmtsRun(rpmts ts, rpmps okProbs, rpmprobFilterFlags ignoreSet) = 0;

    /// @brief Initializes an iterator
    /// @param ts rpmts
    /// @param rpmtag rpmDbiTagVal
    /// @param keypointer keypointer
    /// @param keylen keylen
    /// @return rpmdbMatchIterator
    virtual rpmdbMatchIterator
    rpmtsInitIterator(const rpmts ts, rpmDbiTagVal rpmtag, const void* keypointer, size_t keylen) = 0;

    /// @brief Gets the next header
    /// @param mi rpmdbMatchIterator
    /// @return Header
    virtual Header rpmdbNextIterator(rpmdbMatchIterator mi) = 0;

    /// @brief Frees an iterator
    /// @param mi rpmdbMatchIterator
    /// @return rpmdbMatchIterator
    virtual rpmdbMatchIterator rpmdbFreeIterator(rpmdbMatchIterator mi) = 0;
};
