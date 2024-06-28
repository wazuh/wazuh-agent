#pragma ocne

#include "db_wrapper.hpp"

#include <iostream>
#include <nlohmann/json.hpp>
#include <rocksdb/db.h>
#include <rocksdb/options.h>

using json = nlohmann::json;

class RocksDBWrapper : public DBWrapper
{
public:
    RocksDBWrapper(const std::string& dbPath)
    {
        rocksdb::Options options;
        options.create_if_missing = true;
        rocksdb::Status status = rocksdb::DB::Open(options, dbPath, &db);
        if (!status.ok())
        {
            std::cerr << "Unable to open/create database: " << dbPath << std::endl;
            std::cerr << status.ToString() << std::endl;
            db = nullptr;
        }
    }

    ~RocksDBWrapper() override
    {
        delete db;
    }

    void CreateTable() override {}
    void DropTable() override {}

    void InsertCommand(const std::string& command_data) override
    {
        Command command {commandId, command_data, "pending"};
        std::string key = std::to_string(commandId);
        std::string value = serializeCommand(command);
        db->Put(rocksdb::WriteOptions(), key, value);
        commandId++;
    }

    Command FetchPendingCommand() override
    {
        auto it = db->NewIterator(rocksdb::ReadOptions());
        it->SeekToFirst();

        while (it->Valid())
        {
            auto command = deserializeCommand(it->value().ToString());
            if (command.status == "pending")
            {
                delete it;
                return command;
            }
            it->Next();
        }
        delete it;
        Command command;
        command.id = -1;
        return command;
    }

    void UpdateCommandStatus(int command_id) override
    {
        std::string key = std::to_string(command_id);
        std::string value;
        db->Get(rocksdb::ReadOptions(), key, &value);
        Command command = deserializeCommand(value);
        command.status = "dispatched";
        db->Put(rocksdb::WriteOptions(), key, serializeCommand(command));
    }

    void InsertEvent(int id, const std::string& event_data, const std::string& event_type) override
    {
        Event event {id, event_data, event_type, "pending"};
        std::string key = std::to_string(id);
        std::string value = serializeEvent(event);
        db->Put(rocksdb::WriteOptions(), key, value);
    }

    std::vector<Event> FetchPendingEvents(int limit) override
    {
        std::vector<Event> events;
        auto it = db->NewIterator(rocksdb::ReadOptions());
        for (it->SeekToFirst(); it->Valid() && events.size() < limit; it->Next())
        {
            Event event = deserializeEvent(it->value().ToString());
            if (event.status == "pending")
            {
                events.push_back(event);
            }
        }
        delete it;
        return events;
    }

    std::vector<Event> FetchAndMarkPendingEvents(int limit) override
    {
        std::vector<Event> events;
        rocksdb::WriteBatch batch;
        auto it = db->NewIterator(rocksdb::ReadOptions());
        for (it->SeekToFirst(); it->Valid() && events.size() < limit; it->Next())
        {
            Event event = deserializeEvent(it->value().ToString());
            if (event.status == "pending")
            {
                event.status = "processing";
                events.push_back(event);
                batch.Put(it->key(), serializeEvent(event));
            }
        }
        rocksdb::Status s = db->Write(rocksdb::WriteOptions(), &batch);
        delete it;
        return events;
    }

    void UpdateEventStatus(const std::vector<int>& event_ids, const std::string& status) override
    {
        for (int id : event_ids)
        {
            std::string key = std::to_string(id);
            std::string value;
            db->Get(rocksdb::ReadOptions(), key, &value);
            Event event = deserializeEvent(value);
            event.status = status;
            db->Put(rocksdb::WriteOptions(), key, serializeEvent(event));
        }
    }

    void DeleteEntriesWithStatus(const std::string& status) override
    {
        rocksdb::WriteBatch batch;
        auto it = db->NewIterator(rocksdb::ReadOptions());
        for (it->SeekToFirst(); it->Valid(); it->Next())
        {
            Event event = deserializeEvent(it->value().ToString());
            if (event.status == status)
            {
                batch.Delete(it->key());
            }
        }
        db->Write(rocksdb::WriteOptions(), &batch);
        delete it;
    }

    void UpdateEntriesStatus(const std::string& from_status, const std::string& to_status) override
    {
        rocksdb::WriteBatch batch;
        auto it = db->NewIterator(rocksdb::ReadOptions());
        for (it->SeekToFirst(); it->Valid(); it->Next())
        {
            Event event = deserializeEvent(it->value().ToString());
            if (event.status == from_status)
            {
                event.status = to_status;
                batch.Put(it->key(), serializeEvent(event));
            }
        }
        db->Write(rocksdb::WriteOptions(), &batch);
        delete it;
    }

    int GetPendingEventCount() override
    {
        int count = 0;
        auto it = db->NewIterator(rocksdb::ReadOptions());
        for (it->SeekToFirst(); it->Valid(); it->Next())
        {
            Event event = deserializeEvent(it->value().ToString());
            if (event.status == "pending")
            {
                ++count;
            }
        }
        delete it;
        return count;
    }

private:
    rocksdb::DB* db = nullptr;
    int commandId = 0;

    std::string serializeEvent(const Event& event)
    {
        json j;
        j["id"] = event.id;
        j["event_data"] = event.event_data;
        j["event_type"] = event.event_type;
        j["status"] = event.status;
        return j.dump();
    }

    Event deserializeEvent(const std::string& event_str)
    {
        json j = json::parse(event_str);
        return {j["id"], j["event_data"], j["event_type"], j["status"]};
    }

    std::string serializeCommand(const Command& command)
    {
        json j;
        j["id"] = command.id;
        j["command_data"] = command.command_data;
        j["status"] = command.status;
        return j.dump();
    }

    Command deserializeCommand(const std::string& command_str)
    {
        json j = json::parse(command_str);
        return {j["id"], j["command_data"], j["status"]};
    }
};
