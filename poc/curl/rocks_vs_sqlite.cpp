#include "db/dummy_wrapper.hpp"
#include "db/sqlite_wrapper.hpp"
#include "db/rocksdb_wrapper.hpp"
#include "TimeMeasurement.hpp"

int main()
{
    std::unique_ptr<DBWrapper> pRocks = std::make_unique<RocksDBWrapper>("rocksdb-db");
    std::unique_ptr<DBWrapper> pSql = std::make_unique<SQLiteWrapper>();

    pSql->DropTable();
    pSql->CreateTable();

    {
        TimeMeasurement tm("RocksDB - Inserting events");
        // Preload db with events
        for (int i = 0; i < 10000; ++i)
        {
            pRocks->InsertEvent(i, "event_data", "event_type");
        }
    }

    {
        TimeMeasurement tm("SQLite - Inserting events");
        // Preload db with events
        for (int i = 0; i < 10000; ++i)
        {
            pSql->InsertEvent(i, "event_data", "event_type");
        }
    }

    std::cout << "-----------------------------------" << std::endl;

    {
        TimeMeasurement tm("RocksDB - Reading events");

        int count = pRocks->GetPendingEventCount();
        std::cout << "Found " << count << " pending events." << std::endl;
    }

    {
        TimeMeasurement tm("SQLite - Reading events");

        int count = pSql->GetPendingEventCount();
        std::cout << "Found " << count << " pending events." << std::endl;
    }

    std::cout << "-----------------------------------" << std::endl;

    {
        TimeMeasurement tm("RocksDB - Updating events");

        pRocks->UpdateEntriesStatus("processing", "pending");
    }

    {
        TimeMeasurement tm("SQLite - Updating events");

        pSql->UpdateEntriesStatus("processing", "pending");
    }

    return 0;
}
