#pragma once

#include <string>
#include <vector>

struct Event
{
    int id;
    std::string event_data;
    std::string event_type;
    std::string status;
};

struct Command
{
    int id;
    std::string command_data;
    std::string status;
};

class DBWrapper
{
public:
    virtual ~DBWrapper() = default;

    virtual void CreateTable() = 0;
    virtual void DropTable() = 0;
    virtual void InsertEvent(int id, const std::string& event_data, const std::string& event_type) = 0;
    virtual std::vector<Event> FetchPendingEvents(int limit) = 0;
    virtual std::vector<Event> FetchAndMarkPendingEvents(int limit) = 0;
    virtual void UpdateEventStatus(const std::vector<int>& event_ids, const std::string& status) = 0;
    virtual void DeleteEntriesWithStatus(const std::string& status) = 0;
    virtual void UpdateEntriesStatus(const std::string& from_status, const std::string& to_status) = 0;
    virtual int GetPendingEventCount() = 0;
    virtual void InsertCommand(const std::string& command_data) = 0;
    virtual Command FetchPendingCommand() = 0;
    virtual void UpdateCommandStatus(int commandId) = 0;
};
