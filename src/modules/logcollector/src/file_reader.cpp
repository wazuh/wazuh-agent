#include "file_reader.hpp"
#include <logcollector.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

using namespace logcollector;
using namespace std;

Awaitable FileReader::run() {
    auto & logcollector = Logcollector::Instance();

    string path = "/var/log/syslog";
    auto lf = Localfile(path);
    logcollector.EnqueueTask(lf.run());

    co_return;
}

Localfile::Localfile(string filePath) : m_filePath(std::move(filePath)) { }

Awaitable Localfile::run() {
    auto & logcollector = Logcollector::Instance();
    logcollector.SendMessage("test", "Hello World (from Localfile)");
    co_return;
}
