#pragma once

#include <memory>
#include <string>
#include <vector>

#include <rpm/header.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>

#include "rpmlibWrapper.h"

/// @brief RPM package manager
class RpmPackageManager final
{
public:
    /// @brief Constructor
    /// @param wrapper rpmlib wrapper
    explicit RpmPackageManager(std::shared_ptr<IRpmLibWrapper>&& wrapper);

    /// @brief Destructor
    ~RpmPackageManager();

    /// @brief Package information
    struct Package
    {
        std::string name;
        std::string version;
        std::string release;
        uint64_t epoch = 0;
        std::string summary;
        std::string installTime;
        uint64_t size = 0;
        std::string vendor;
        std::string group;
        std::string source;
        std::string architecture;
        std::string description;
    };

    /// @brief Iterator for packages
    struct Iterator final
    {
        bool operator!=(const Iterator& other)
        {
            return m_end != other.m_end;
        };

        void operator++();
        Package operator*();
        ~Iterator();

    private:
        // Used for end iterator
        Iterator();
        // Used for regular iterator
        Iterator(std::shared_ptr<IRpmLibWrapper>& rpmlib);
        std::string getAttribute(rpmTag tag) const;
        uint64_t getAttributeNumber(rpmTag tag) const;
        bool m_end = false;
        std::shared_ptr<IRpmLibWrapper> m_rpmlib;
        rpmts m_transactionSet = nullptr;
        rpmdbMatchIterator m_matches = nullptr;
        rpmtd m_dataContainer = nullptr;
        Header m_header = nullptr;
        friend class RpmPackageManager;
    };

    static const Iterator END_ITERATOR;

    /// @brief Iterator for packages starting from the beginning
    Iterator begin()
    {
        return Iterator {m_rpmlib};
    }

    /// @brief Iterator for packages at the end
    const Iterator& end() const
    {
        return END_ITERATOR;
    }

private:
    static bool ms_instantiated;
    std::shared_ptr<IRpmLibWrapper> m_rpmlib;
};
