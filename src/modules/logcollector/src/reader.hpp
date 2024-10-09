#pragma once

#include <boost/asio/awaitable.hpp>

namespace logcollector {

using Awaitable = boost::asio::awaitable<void>;

class IReader {
public:
    virtual ~IReader() = default;
    virtual Awaitable run() = 0;
};

}
