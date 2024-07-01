#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <sqlite3.h>
#include "TimeMeasurement.hpp"
#include "terminal.hpp"

// Thread-safe queue implementation
template <typename T>
class EventQueue 
{
public:
    void push(T value) 
    {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(std::move(value));
        cond_var.notify_one();
    }

    T pop() 
    {
        std::unique_lock<std::mutex> lock(mutex);
        cond_var.wait(lock, [this] { return !queue.empty(); });
        T value = std::move(queue.front());
        queue.pop();
        return value;
    }

    bool isEmpty()
    {
        //std::lock_guard<std::mutex> lock(mutex);
        return queue.size() == 0;
    }

    size_t size()
    {
        //std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }

private:
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable cond_var;
};

enum class EventAction
{
    EventActionInsert,
    EventActionUpdate,
    EventActionDelete
};

class Event
{
public:
    Event() : id{0}, data{""}, type{""}, timestamp{""}, status{""} , action{EventAction::EventActionInsert}
    {

    };

    Event(int i, std::string d, std::string t, std::string ts, std::string st, EventAction ea)
     : id{i}, data{d}, type{t}, timestamp{ts}, status{st}, action{ea}
     {

     };

     Event(const Event& e)
     : id{e.id}, data{e.data}, type{e.type}, timestamp{e.timestamp}, status{e.status}, action{e.action}
     {
     
     }

     Event& operator=(const Event& e) 
     {
        id = e.id;
        data = e.data;
        type = e.type;
        timestamp = e.timestamp;
        status = e.status;
        action = e.action;

        return *this;
     }

    long id;
    std::string data;
    std::string type;
    std::string timestamp;
    std::string status;
    EventAction action;
};

class SQLiteDB
{
public:
    SQLiteDB()
    {
        DropTable();
    }

    ~SQLiteDB()
    {
        sqlite3_close(mDB);
    }

    void CreateTable()
        {
            const char* sql = "CREATE TABLE IF NOT EXISTS events ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                            "event_data TEXT NOT NULL, "
                            "event_type TEXT NOT NULL, "
                            "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
                            "status TEXT DEFAULT 'pending'"
                            ");";
            char* errMsg = nullptr;
            int rc = sqlite3_exec(mDB, sql, nullptr, nullptr, &errMsg);
            if (rc != SQLITE_OK)
            {
                std::cerr << "Error creating table: " << errMsg << std::endl;
                sqlite3_free(errMsg);
            }
        }

    int OpenDatabase()
    {
        if(mDB != nullptr)
        {
            sqlite3_close(mDB);
            mDB = nullptr;
        }

        if (sqlite3_open("test.db", &mDB)) 
        {
            std::cerr << "Can't open database: " << sqlite3_errmsg(mDB) << std::endl;
            return 1;
        }

        return 0;
    }

    void DropTable()
    {
        if(mDB == nullptr)
            return;

        const char* sql = "DROP TABLE IF EXISTS events;";
        char* errMsg = nullptr;
        int rc = sqlite3_exec(mDB, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK)
        {
            std::cerr << "Error dropping table: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        } 
    }

    void BeginTransaction()
    {
        if(mDB == nullptr)
            return;   
        if(mTransactionInProgress)
            return;

        writeMutex.lock();

        char* errMsg = nullptr;
        sqlite3_exec(mDB, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);
        mTransactionInProgress = true;
    }

    void CommitTransaction()
    {
        if(mDB == nullptr)
            return;

        if(!mTransactionInProgress)
            return;

        char* errMsg = nullptr;
        sqlite3_exec(mDB, "COMMIT;", nullptr, nullptr, &errMsg);

        writeMutex.unlock();

        mTransactionInProgress = false;
    }

    bool HandleEvent(const Event& e)
    {   
        if(mDB == nullptr)
            return false;

        // build the SQL statement
        std::string sql;
        switch (e.action)
        {       
            case EventAction::EventActionInsert:
                sql = "INSERT INTO events (event_data, event_type, status) VALUES ('" + e.data + "', '" + e.type + "', '" + e.status + "');";
                break;
            case EventAction::EventActionUpdate:
                sql = "UPDATE events SET event_data='" + e.data + "', event_type='" + e.type + "', status='" + e.status + "' WHERE id=" + std::to_string(e.id) + ";";
                break;
            case EventAction::EventActionDelete:
                sql = "DELETE FROM events WHERE id=" + std::to_string(e.id) + ";";
                break;
                
        }

        //std::cout << sql << std::endl;

        // now exectute the statement
        if (char* errMsg = nullptr; sqlite3_exec(mDB, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) 
        {
            std::cerr << "SQL error: " << errMsg << std::endl;
            //std::cout << sql << std::endl;

            sqlite3_free(errMsg);
            return false;
        }

        return true;
    }

    std::vector<Event> FetchPendingEvents(int limit)
    {
        std::vector<Event> events;

        const char* sql = "SELECT * FROM events WHERE status = 'Pending' LIMIT ?;";
        
        if (mDB == nullptr)
            return events;

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(mDB, sql, -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, limit);

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            Event e;

            e.id = sqlite3_column_int(stmt, 0);

            e.data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            e.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            e.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            e.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            events.emplace_back(e);
        }
        sqlite3_finalize(stmt);
        return events;
    }
    
    sqlite3* GetDB(){ return mDB; };

private:
    bool mTransactionInProgress {false};
    sqlite3* mDB {nullptr};
    std::mutex writeMutex;
};

class InsertParams
{
public:
    int maxItems;
    double maxTime;
    int batchSize;
};

// Function to handle items in the queue
void insertEvents(EventQueue<Event>& eQueue, SQLiteDB& db, InsertParams& params) 
{

    Terminal::GetTerminal().printToCoordinates(0, 3, "Inserter: ");

    long nItemCount {0};
    long nBatchCount {0};
    
    TimeMeasurement tm("Inserter thread");

    // go on forever
    while (true) 
    {   
        // check for test max items
        if (nItemCount >= params.maxItems)
            break;

        // check for test max time
        if (tm.GetElapsedMs() > params.maxTime)
            break;

        // if queue is empty sleep for a while
        if (eQueue.isEmpty())
        {
            Terminal::GetTerminal().printToCoordinates(11, 3, std::string("Queue size:") + std::to_string(eQueue.size()));
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        // run a batch until the queue is empty or we reach the batch size
        db.BeginTransaction();
        while (!eQueue.isEmpty())
        {
            Event item = eQueue.pop();

            db.HandleEvent(item);
            nItemCount++;
            nBatchCount++;

            // upate output info
            Terminal::GetTerminal().printToCoordinates(11, 3, std::string("Queue size:") + std::to_string(eQueue.size()));
            Terminal::GetTerminal().printToCoordinates(30, 3, std::string("Handled: ") + std::to_string(nItemCount));
            
            if (nBatchCount > params.batchSize)
            {
                nBatchCount = 0;
                break;   // continue to next batch
            }
        }
        db.CommitTransaction();
    }

    Terminal::GetTerminal().printToCoordinates(30, 3, std::string("Handled: ") + std::to_string(nItemCount));
    Terminal::GetTerminal().printToCoordinates(60, 3, std::string("Thread duration: ") + std::to_string(tm.GetElapsedMs()));

}

// Function to produce events to be inserted in the DB
void generateEvents(EventQueue<Event>& eQueue, int numItems, double maxTime) 
{

    Terminal::GetTerminal().printToCoordinates(0, 2, "Generator: ");

    TimeMeasurement tm("Generator thread");

    for (int i = 0; i < numItems; ++i) 
    {
        if(tm.GetElapsedMs() > maxTime)
            break;

        eQueue.push(Event {i, "{\"Some random json data\",\"165436\",\"Some string here\",\"436.4321\"}", "Event", "", "Pending", EventAction::EventActionInsert});
        Terminal::GetTerminal().printToCoordinates(11, 2, std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    Terminal::GetTerminal().printToCoordinates(60, 2, std::string("Thread duration: ") + std::to_string(tm.GetElapsedMs()));
}

// Function to read events from the DB, do something with them and update them as processed
void readAndUpdateEvents(SQLiteDB& db, int numItems, double maxTime) 
{

    Terminal::GetTerminal().printToCoordinates(0, 4, "Reader/updater: ");

    TimeMeasurement tm("Reader/udater thread");

    int itemCount {0};

    while (true) 
    {
        if(tm.GetElapsedMs() > maxTime)
            break;

        if (itemCount > numItems)
            break;

        std::vector<Event>evs = db.FetchPendingEvents(1);
        if (evs.size() == 0)
        {
            //std::cout << "No events" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        for (Event e : evs)
        {
            //DoSomethingWithEvent();

            // Update the event in the database
            e.status = "Processed";
            e.action = EventAction::EventActionUpdate;
            db.HandleEvent(e);

            itemCount++;
            Terminal::GetTerminal().printToCoordinates(18, 4, std::to_string(itemCount));

        }


    }
    Terminal::GetTerminal().printToCoordinates(60, 4, std::string("Thread duration: ") + std::to_string(tm.GetElapsedMs()));
}

#define MAXSECONDS 30
#define MAXEVENTS 10'000'000
#define TRANSACTION_SIZE 10

int main() 
{
    SQLiteDB db;

    int x, y;
    //Terminal::GetTerminal().GetCursorPos(&y, &x);
    Terminal::GetTerminal().Clear();
    //Terminal::GetTerminal().printToCoordinates(0, 0, "--------- OPENING DATABASE -----------");

    db.OpenDatabase();
    db.CreateTable();

    EventQueue<Event> eventQueue;

    TimeMeasurement tm("Threads duration");

    // Create threads
    std::thread producer(generateEvents, std::ref(eventQueue), MAXEVENTS, MAXSECONDS * 1000.0);

    InsertParams params { MAXEVENTS, MAXSECONDS * 1000.0, TRANSACTION_SIZE};
    std::thread consumer(insertEvents, std::ref(eventQueue), std::ref(db), std::ref(params));

    std::thread updater(readAndUpdateEvents, std::ref(db), MAXEVENTS, MAXSECONDS * 1000.0);

    // Join threads
    producer.join();
    consumer.join();
    updater.join();

    Terminal::GetTerminal().printToCoordinates(0, 6, std::string("Threads duration: ") + std::to_string(tm.GetElapsedMs()) + "\n");
    return 0;
}
