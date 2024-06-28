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


class Event
{
public:
    Event() : id{""}, data{""}, type{""}, timestamp{""}, status{""} {};

    Event(std::string i, std::string d, std::string t, std::string ts, std::string st)
     : id{i}, data{d}, type{t}, timestamp{ts}, status{st} 
     {

     };

     Event(const Event& e)
     : id{e.id}, data{e.data}, type{e.type}, timestamp{e.timestamp}, status{e.status} 
     {
     
     }

     Event& operator=(const Event& e) 
     {
        id = e.id;
        data = e.data;
        type = e.type;
        timestamp = e.timestamp;
        status = e.status;

        return *this;
     }

    std::string id;
    std::string data;
    std::string type;
    std::string timestamp;
    std::string status;
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
        if(mTransactionInProgress)
            return;

        if(mDB == nullptr)
            return;

        char* errMsg = nullptr;
        sqlite3_exec(mDB, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);
        mTransactionInProgress = true;
    }

    void CommitTransaction()
    {
        if(!mTransactionInProgress)
            return;

        if(mDB == nullptr)
            return;
            
        char* errMsg = nullptr;
        sqlite3_exec(mDB, "COMMIT;", nullptr, nullptr, &errMsg);
        mTransactionInProgress = false;
    }

    sqlite3* GetDB(){ return mDB; };

private:
    bool mTransactionInProgress {false};
    sqlite3* mDB {nullptr};
};

class InsertParams
{
public:
    int maxItems;
    double maxTime;
    int batchSize;
};

// Function to insert items into the database
void insertEvents(EventQueue<Event>& eQueue, SQLiteDB& db, InsertParams& params) 
{

    Terminal::GetTerminal().printToCoordinates(0, 3, "Insertor: ");

    db.BeginTransaction();
    long nItemCount {0};
    long nBatchCount {0};
    
    TimeMeasurement tm("Insertor thread");

    while (true) 
    {
        if (nItemCount >= params.maxItems)
            break;

        if (tm.GetElapsedMs() > params.maxTime)
            break;

        if (eQueue.isEmpty())
        {
            //Terminal::GetTerminal().printToCoordinates(11, 3, "Queue is empty");
            Terminal::GetTerminal().printToCoordinates(11, 3, std::string("Queue size:") + std::to_string(eQueue.size()));
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        if (nBatchCount > params.batchSize)
        {
            db.CommitTransaction();
            db.BeginTransaction();
            nBatchCount = 0;
        }

        Event item = eQueue.pop();

        Terminal::GetTerminal().printToCoordinates(11, 3, std::string("Queue size:") + std::to_string(eQueue.size()));
        Terminal::GetTerminal().printToCoordinates(30, 3, std::string("Inserted: ") + std::to_string(nItemCount));

        std::string sql = "INSERT INTO events (event_data, event_type, status) VALUES ('" + item.data + "', '" + item.type + "', '" + item.status + "');";
        if (char* errMsg = nullptr; sqlite3_exec(db.GetDB(), sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) 
        {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }

        nItemCount++;
        nBatchCount++;
    }

    db.CommitTransaction();

    Terminal::GetTerminal().printToCoordinates(30, 3, std::string("Inserted: ") + std::to_string(nItemCount));
    Terminal::GetTerminal().printToCoordinates(60, 3, std::string("Thread duration: ") + std::to_string(tm.GetElapsedMs()));

}

// Function to produce items (simulating data generation)
void generateEvents(EventQueue<Event>& eQueue, int numItems, double maxTime) 
{

    Terminal::GetTerminal().printToCoordinates(0, 2, "Generator: ");

    TimeMeasurement tm("Generator thread");

    for (int i = 0; i < numItems; ++i) 
    {
        if(tm.GetElapsedMs() > maxTime)
            break;

        eQueue.push(Event {std::to_string(i), "{\"Some random json data\",\"165436\",\"Some string here\",\"436.4321\"}", "Event", "", "Pending"});
        Terminal::GetTerminal().printToCoordinates(11, 2, std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    Terminal::GetTerminal().printToCoordinates(60, 2, std::string("Thread duration: ") + std::to_string(tm.GetElapsedMs()));
}


int main() 
{
    SQLiteDB db;

    int x, y;
    //Terminal::GetTerminal().GetCursorPos(&y, &x);
    Terminal::GetTerminal().Clear();
    Terminal::GetTerminal().printToCoordinates(0, 0, "--------- OPENING DATABASE -----------");

    db.OpenDatabase();
    db.CreateTable();


    EventQueue<Event> eventQueue;

    TimeMeasurement tm("Threads duration");

    // Create threads
    std::thread producer(generateEvents, std::ref(eventQueue), 5'000'000, 60000.0);

    InsertParams params { 10'000'000, 60500.0, 10};
    std::thread consumer(insertEvents, std::ref(eventQueue), std::ref(db), std::ref(params));

    // Join threads
    producer.join();
    consumer.join();

    Terminal::GetTerminal().printToCoordinates(0, 4, std::string("Threads duration: ") + std::to_string(tm.GetElapsedMs()) + "\n");
    return 0;
}
