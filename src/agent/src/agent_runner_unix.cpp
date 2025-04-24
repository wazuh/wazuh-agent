#include <agent_runner.hpp>

#include <config.h>
#include <configuration_parser.hpp>

#include <boost/asio.hpp>
#include <fmt/format.h>

#include <iostream>

void AgentRunner::AddPlatformSpecificOptions() {}

std::optional<int> AgentRunner::HandlePlatformSpecificOptions() const
{
    return std::nullopt;
}

bool AgentRunner::SendSignal(const std::string& message, const std::string& configFilePath) const
{
    boost::asio::io_context ioContext;
    boost::asio::local::stream_protocol::socket socket(ioContext);
    boost::system::error_code ec;

    const auto configurationParser = configFilePath.empty()
                                         ? configuration::ConfigurationParser()
                                         : configuration::ConfigurationParser(std::filesystem::path(configFilePath));

    const auto runPath = configurationParser.GetConfigOrDefault(config::DEFAULT_RUN_PATH, "agent", "path.run");

    const std::string socketPath = fmt::format("{}/agent-socket", runPath);

    // NOLINTNEXTLINE(bugprone-unused-return-value)
    socket.connect(boost::asio::local::stream_protocol::endpoint(socketPath), ec);

    if (!ec)
    {
        std::string command = message + '\n'; // Add newline for read_until
        boost::asio::write(socket, boost::asio::buffer(command), ec);
        if (ec)
        {
            std::cout << "Client write error: " << ec.message() << '\n';
            return false;
        }

        std::cout << "Client sent signal: " << message << '\n';
        return true;
    }
    else
    {
        std::cout << "Client connect error: " << ec.message() << '\n';
        return false;
    }
}
