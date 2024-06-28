#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "db_wrapper.hpp"
#include "logger.hpp"
#include "std_thread_manager.hpp"

struct EventQueueMonitor
{
    EventQueueMonitor(std::unique_ptr<DBWrapper> pdb, std::function<bool(const std::string&)> onEvent)
    : eventQueue(std::move(pdb))
    {
        eventQueue->CreateTable();
        eventQueue->UpdateEntriesStatus("processing", "pending");

        Logger::log("EVENT QUEUE MONITOR", "Starting event queue thread");
        dispatcher_thread = std::make_unique<std::thread>([this, onEvent]() { Run(onEvent); });
    }

    ~EventQueueMonitor()
    {
        continueEventProcessing.store(false);
        Logger::log("EVENT QUEUE MONITOR", "Waiting for event queue thread to join");

        if (dispatcher_thread && dispatcher_thread->joinable())
        {
            dispatcher_thread->join();
        }

        dispatcher_thread.reset();

        PerformCleanup();

        eventQueue.reset();

        Logger::log("EVENT QUEUE MONITOR", "Destroyed");
    }

    void Run(std::function<bool(const std::string&)> onEvent)
    {
        auto last_dispatch_time = std::chrono::steady_clock::now();

        while (continueEventProcessing.load())
        {
            // This clean up should be performed on another thread, possibly by the ThreadManager
            // otherwise we are creating threads and we immediately wait for them to finish
            // making the thread creation pointless
            // PerformCleanup();

            if (!ShouldDispatchEvents(last_dispatch_time))
            {
                if (shouldStopRunningIfQueueIsEmpty)
                {
                    // Stop event loop if no more events are found (useful for testing)
                    continueEventProcessing.store(false);
                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                continue;
            }

            if (auto pending_events = eventQueue->FetchAndMarkPendingEvents(batchSize); !pending_events.empty())
            {
                DispatchPendingEvents(onEvent, pending_events);
            }

            last_dispatch_time = std::chrono::steady_clock::now();
        }
    }

    void PerformCleanup()
    {
        threadManager.CleanUpJoinableThreads();
        CleanUpDispatchedEvents();
    }

    bool ShouldDispatchEvents(const std::chrono::time_point<std::chrono::steady_clock>& last_dispatch_time)
    {
        const auto current_time = std::chrono::steady_clock::now();

        // The dispatch interval may not be necessary during PoC testing
        [[maybe_unused]] const auto hasDispatchIntervalPassed = std::chrono::duration_cast<std::chrono::seconds>(current_time - last_dispatch_time).count() >=
                   dispatchInterval;

        return eventQueue->GetPendingEventCount() > 0;
    }

    void DispatchPendingEvents(const std::function<bool(const std::string&)>& onEvent,
                               const std::vector<Event>& pending_events)
    {
        std::vector<int> event_ids;
        std::string event_data = "[";

        for (size_t i = 0; i < pending_events.size(); ++i)
        {
            const auto& event = pending_events[i];

            Logger::log("EVENT QUEUE MONITOR",
                        "Dispatching event ID: " + std::to_string(event.id) + ", Data: " + event.event_data);

            event_ids.push_back(event.id);
            event_data += event.event_data;

            if (i != pending_events.size() - 1)
            {
                event_data += ",";
            }
        }

        event_data += "]";

        // Create a new thread for each event batch to dispatch
        threadManager.CreateThread([this, onEvent, event_data, event_ids]()
                                   { UpdateEventStatus(onEvent(event_data), event_ids); });
    }

    void UpdateEventStatus(bool success, const std::vector<int>& event_ids)
    {
        if (success)
        {
            eventQueue->UpdateEventStatus(event_ids, "dispatched");
        }
        else
        {
            eventQueue->UpdateEventStatus(event_ids, "pending");
        }
    }

    void CleanUpDispatchedEvents()
    {
        // Cleanup dispatched events
        // (this could be done in a separate thread as well, maybe by a different class)
        eventQueue->DeleteEntriesWithStatus("dispatched");
    }

    std::atomic<bool> continueEventProcessing = true;
    std::unique_ptr<std::thread> dispatcher_thread;
    std::unique_ptr<DBWrapper> eventQueue;
    StdThreadManager threadManager;

    // Configuration constants
    bool shouldStopRunningIfQueueIsEmpty = false;
    int batchSize = 10;       // Number of events to dispatch at once
    int dispatchInterval = 5; // Time interval in seconds
};
