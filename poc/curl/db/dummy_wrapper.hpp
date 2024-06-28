#pragma ocne

#include "db_wrapper.hpp"

#include <iostream>
#include <atomic>
#include <vector>
#include <algorithm>
#include <mutex>


class DummyWrapper : public DBWrapper
{
public:
    ~DummyWrapper() override
    {
    }

    void CreateTable() override
    {
    }

    void InsertEvent(int id, const std::string& event_data, const std::string& event_type) override
    {
        events.emplace_back(Event {id, "event_data", "event_type", "pending"});
        pending++;
    }

    std::vector<Event> FetchPendingEvents(int limit) override
    {
        std::lock_guard<std::mutex> lock(events_mutex);

        if (pending.load() == 0)
        {
            return {};
        }

        limit = std::min(limit, pending.load());
        pending.fetch_sub(limit);

        std::vector<Event> result(events.begin(), events.begin() + limit);
        return result;
    }

    std::vector<Event> FetchAndMarkPendingEvents(int limit) override
    {
        std::lock_guard<std::mutex> lock(events_mutex);

        if (pending.load() == 0)
        {
            return {};
        }

        limit = std::min(limit, pending.load());
        pending.fetch_sub(limit);
        processing.fetch_add(limit);

        std::vector<Event> result(events.begin(), events.begin() + limit);
        return result;
    }

    void UpdateEventStatus(const std::vector<int>& event_ids, const std::string& status) override
    {
        if (status == "dispatched")
        {
            processing.fetch_sub(event_ids.size());
            dispatched.fetch_add(event_ids.size());
        }
        if (status == "processing")
        {
            pending.fetch_sub(event_ids.size());
            processing.fetch_add(event_ids.size());
        }
        if (status == "pending")
        {
            processing.fetch_sub(event_ids.size());
            pending.fetch_add(event_ids.size());
        }
    }

    void DeleteEntriesWithStatus(const std::string& status) override
    {
        if (status == "dispatched")
        {
            dispatched = 0;
        }
        if (status == "processing")
        {
            processing = 0;
        }
        if (status == "pending")
        {
            pending = 0;
        }
    }

    void UpdateEntriesStatus(const std::string& from_status, const std::string& to_status) override
    {
        if (from_status == "dispatched" && to_status == "processing")
        {
            processing.fetch_add(dispatched);
            dispatched = 0;
        }
        if (from_status == "dispatched" && to_status == "pending")
        {
            pending.fetch_add(dispatched);
            dispatched = 0;
        }
        if (from_status == "processing" && to_status == "pending")
        {
            pending.fetch_add(processing);
            processing = 0;
        }
        if (from_status == "processing" && to_status == "dispatched")
        {
            dispatched.fetch_add(processing);
            processing = 0;
        }
        if (from_status == "pending" && to_status == "processing")
        {
            processing.fetch_add(pending);
            pending = 0;
        }

        if (from_status == "pending" && to_status == "dispatched")
        {
            dispatched.fetch_add(pending);
            pending = 0;
        }
    }

    int GetPendingEventCount() override
    {
        return pending;
    }

    void InsertCommand(const std::string& command_data) override {}

    Command FetchPendingCommand()
    {
        Command command;
        return command;
    }

    void UpdateCommandStatus(int commandId) {}

private:
    std::atomic<int> pending = 0;
    std::atomic<int> processing = 0;
    std::atomic<int> dispatched = 0;
    std::vector<Event> events;
    std::mutex events_mutex;
};
