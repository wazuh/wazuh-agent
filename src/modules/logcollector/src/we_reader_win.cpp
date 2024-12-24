#include "we_reader_win.hpp"

#include <logger.hpp>

using namespace logcollector;

WindowsEventTracerReader::WindowsEventTracerReader(Logcollector &logcollector,
                           const std::list<std::string> channels,
                           const std::list<std::string> queries,
                           const std::time_t reloadInterval,
                           bool bookmarkEnabled) :
    IReader(logcollector),
    m_channelsList(channels),
    m_queriesList(queries),
    m_ReaderReloadInterval(reloadInterval),
    m_bookmarkEnabled(bookmarkEnabled) { }

Awaitable WindowsEventTracerReader::Run()
{
    // TODO: add lambda for loop break
    while (true) {
        m_logcollector.EnqueueTask(ReadEventChannel());

        co_await m_logcollector.Wait(
            std::chrono::milliseconds(m_ReaderReloadInterval));
    }
}

Awaitable WindowsEventTracerReader::ReadEventChannel()
{
    for (auto eventChannel : m_channelsList)
    { 
        const std::string log {};
        m_logcollector.SendMessage(eventChannel, log, m_collectorType);

        co_await m_logcollector.Wait(std::chrono::milliseconds(m_ReaderReloadInterval));
    }
}

