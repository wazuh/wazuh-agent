#pragma once

#include "iberkeleyDbWrapper.h"
#include <stdexcept>

/// @brief Deleter for Berkeley DB
struct BerkeleyRpmDbDeleter final
{
    void operator()(DB* db)
    {
        db->close(db, 0);
    }

    void operator()(DBC* cursor)
    {
        cursor->c_close(cursor);
    }
};

/// @brief Wrapper for Berkeley DB
class BerkeleyDbWrapper final : public IBerkeleyDbWrapper
{
private:
    std::unique_ptr<DB, BerkeleyRpmDbDeleter> m_db;
    std::unique_ptr<DBC, BerkeleyRpmDbDeleter> m_cursor;

public:
    /// @copydoc IBerkeleyDbWrapper::getRow
    int32_t getRow(DBT& key, DBT& data) override
    {
        std::memset(&key, 0, sizeof(DBT));
        std::memset(&data, 0, sizeof(DBT));
        return m_cursor->c_get(m_cursor.get(), &key, &data, DB_NEXT);
    }

    /// @brief Default destructor
    ~BerkeleyDbWrapper() = default;

    /// @brief Default constructor
    /// @param directory Directory to open
    explicit BerkeleyDbWrapper(const std::string& directory)
    {
        int ret;
        DB* dbp;
        DBC* cursor;

        if ((ret = db_create(&dbp, nullptr, 0)) != 0)
        {
            throw std::runtime_error {db_strerror(ret)};
        }

        m_db = std::unique_ptr<DB, BerkeleyRpmDbDeleter>(dbp);

        // Set Big-endian order by default
        m_db->set_lorder(m_db.get(), 1234);

        if ((ret = m_db->open(m_db.get(), nullptr, directory.c_str(), nullptr, DB_HASH, DB_RDONLY, 0)) != 0)
        {
            throw std::runtime_error {std::string("Failed to open database '") + directory + "': " + db_strerror(ret)};
        }

        if ((ret = m_db->cursor(m_db.get(), nullptr, &cursor, 0)) != 0)
        {
            throw std::runtime_error {std::string("Error creating cursor: ") + db_strerror(ret)};
        }

        m_cursor = std::unique_ptr<DBC, BerkeleyRpmDbDeleter>(cursor);
    }
};
