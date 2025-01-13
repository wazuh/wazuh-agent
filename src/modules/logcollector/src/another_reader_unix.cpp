#include <sstream>
#include <iomanip>

#include <logger.hpp>
#include <logcollector.hpp>

namespace logcollector {

//TODO: Delete once the unix implementation is ready
void AddPlatformSpecificReader([[maybe_unused]] std::shared_ptr<const configuration::ConfigurationParser> configurationParser, [[maybe_unused]] Logcollector &logcollector)
{

}

}