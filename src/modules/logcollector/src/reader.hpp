#pragma once

#include <boost/asio/awaitable.hpp>

namespace logcollector {

using Awaitable = boost::asio::awaitable<void>;

class IReader {
public:
    IReader(Logcollector& logcollector) :
        m_logcollector(logcollector) { }

    virtual ~IReader() = default;
    virtual Awaitable Run() = 0;

protected:
    Logcollector& m_logcollector;
};

}
