#include "event_queue_monitor.hpp" // Adjust the include path as necessary
#include <atomic>
#include <benchmark/benchmark.h>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "db/dummy_wrapper.hpp"
#include "db/sqlite_wrapper.hpp"
#include "db/rocksdb_wrapper.hpp"
#include "TimeMeasurement.hpp"

#include "logger.hpp"

#define ROCKSDB

class EventQueueMonitorFixture : public benchmark::Fixture
{
public:
    EventQueueMonitorFixture()
    {
        Logger::LOGGING_ENABLED = false;
        #ifdef ROCKSDB
            std::cout << "RocksDB Database" << std::endl;
        #else
            std::cout << "SQLite Database" << std::endl;
        #endif
    }

    std::unique_ptr<EventQueueMonitor> monitor;
};

BENCHMARK_DEFINE_F(EventQueueMonitorFixture, Dispatch200000PreLoadedPendingEventsWithBatchSizesOf)
(benchmark::State& state)
{
    std::atomic<int> counter = 0;

    for (auto _ : state)
    {
        #ifdef ROCKSDB
            std::unique_ptr<DBWrapper> p = std::make_unique<RocksDBWrapper>("rocksdb-db");
        #else
            std::unique_ptr<DBWrapper> p = std::make_unique<SQLiteWrapper>();
        #endif
        monitor = std::make_unique<EventQueueMonitor>(std::move(p), [](const std::string&) { return false; });

        // First we stop the event loop so we can control that it starts with a preloaded db
        monitor->continueEventProcessing = false;

        // When all events are processed we stop the event loop
        monitor->shouldStopRunningIfQueueIsEmpty = true;
        
        {
            TimeMeasurement tm("Inserting events");
            // Preload db with events
            for (int i = 0; i < 100; ++i)
            {
                monitor->eventQueue->InsertEvent(i, "event_data", "event_type");
            }
        }

        // Benchmark the time it takes to dispatch all the events
        auto start = std::chrono::high_resolution_clock::now();
        monitor->batchSize = state.range(0);
        monitor->continueEventProcessing = true;
        monitor->Run(
            [&counter](const std::string& event_data) -> bool
            {
                // Avoid optimizing out the counter
                // maybe this is not necessary
                benchmark::DoNotOptimize(counter);
                counter++;
                benchmark::ClobberMemory();
                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
                return true;
            });

        // We include the reset as part of the iteration time so we wait
        // for all the threads to finish
        monitor->threadManager.JoinAll();
        auto end = std::chrono::high_resolution_clock::now();
        monitor.reset();

        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed_seconds.count());
    }
}

BENCHMARK_REGISTER_F(EventQueueMonitorFixture, Dispatch200000PreLoadedPendingEventsWithBatchSizesOf)
    ->UseManualTime()
    ->Unit(benchmark::kMillisecond)
    ->Iterations(10)
    ->Arg(10)
    ->Arg(20)
    ->Arg(50);


BENCHMARK_DEFINE_F(EventQueueMonitorFixture, DispatchPreLoadedPendingEventsWithFixedBatchSizeOf100AndEventCountOf)
(benchmark::State& state)
{
    for (auto _ : state)
    {
        #ifdef ROCKSDB
            std::unique_ptr<DBWrapper> p = std::make_unique<RocksDBWrapper>("rocksdb-db");
        #else
            std::unique_ptr<DBWrapper> p = std::make_unique<SQLiteWrapper>();
        #endif
        monitor = std::make_unique<EventQueueMonitor>(std::move(p), [](const std::string&) { return false; });

        // First we stop the event loop so we can control that it starts with a preloaded db
        monitor->continueEventProcessing = false;

        // When all events are processed we stop the event loop
        monitor->shouldStopRunningIfQueueIsEmpty = true;
        
        {
            TimeMeasurement tm("Inserting events");
            // Preload db with events
            for (int i = 0; i < state.range(0); ++i)
            {
                monitor->eventQueue->InsertEvent(i, "event_data", "event_type");
            }
        }
        std::cout << "Running. " << std::endl;
        
        // Benchmark the time it takes to dispatch all the events
        auto start = std::chrono::high_resolution_clock::now();
        monitor->batchSize = 100;
        monitor->continueEventProcessing = true;
        monitor->Run([](const std::string& event_data) -> bool { return true; });

        // We include the reset as part of the iteration time so we wait
        // for all the threads to finish
        monitor->threadManager.JoinAll();
        auto end = std::chrono::high_resolution_clock::now();
        monitor.reset();

        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed_seconds.count());
    }
}

BENCHMARK_REGISTER_F(EventQueueMonitorFixture, DispatchPreLoadedPendingEventsWithFixedBatchSizeOf100AndEventCountOf)
    ->UseManualTime()
    ->Unit(benchmark::kMillisecond)
    ->Iterations(10)
    ->Arg(1000)
    ->Arg(2000)
    ->Arg(5000);

BENCHMARK_DEFINE_F(EventQueueMonitorFixture, Dispatch100000PreLoadedPendingEventsWithFixedBatchSizeOf100AndEventDataSizeOf)(benchmark::State& state)
{
    // Function to generate a string of N bytes
    auto generateString = [](size_t length, char fill_char = 'A')
    {
        return std::string(length, fill_char);
    };

    for (auto _ : state)
    {
        #ifdef ROCKSDB
            std::unique_ptr<DBWrapper> p = std::make_unique<RocksDBWrapper>("rocksdb-db");
        #else
            std::unique_ptr<DBWrapper> p = std::make_unique<SQLiteWrapper>();
        #endif
        monitor = std::make_unique<EventQueueMonitor>(std::move(p), [](const std::string&) { return false; });

        // First we stop the event loop so we can control that it starts with a preloaded db
        monitor->continueEventProcessing = false;

        // When all events are processed we stop the event loop
        monitor->shouldStopRunningIfQueueIsEmpty = true;

        // Preload db with events
        for (int i = 0; i < 5000; ++i)
        {
            monitor->eventQueue->InsertEvent(i, generateString(state.range(0)), "event_type");
        }

        // Benchmark the time it takes to dispatch all the events
        auto start = std::chrono::high_resolution_clock::now();
        monitor->batchSize = 100;
        monitor->continueEventProcessing = true;
        monitor->Run(
            [](const std::string& event_data) -> bool
            {
                return true;
            });

        // We include the reset as part of the iteration time so we wait
        // for all the threads to finish
        monitor->threadManager.JoinAll();
        auto end = std::chrono::high_resolution_clock::now();
        monitor.reset();

        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed_seconds.count());
    }
}

BENCHMARK_REGISTER_F(EventQueueMonitorFixture, Dispatch100000PreLoadedPendingEventsWithFixedBatchSizeOf100AndEventDataSizeOf)
    ->UseManualTime()
    ->Unit(benchmark::kMillisecond)
    ->Iterations(10)
    ->Arg(100)
    ->Arg(200)
    ->Arg(500);

BENCHMARK_MAIN();
