#include <agent_runner.hpp>

#include <config.h>

#include <boost/asio.hpp>

#include <iostream>

void AgentRunner::AddPlatformSpecificOptions() {}

std::optional<int> AgentRunner::HandlePlatformSpecificOptions() const
{
    return std::nullopt;
}

bool SendSignal(const std::string& message)
{
    boost::asio::io_context ioContext;
    boost::asio::local::stream_protocol::socket socket(ioContext);
    boost::system::error_code ec;

    const std::string socketPath = std::string(config::DEFAULT_RUN_PATH) + "/agent-socket";

    // NOLINTNEXTLINE(bugprone-unused-return-value)
    socket.connect(boost::asio::local::stream_protocol::endpoint(socketPath), ec);

    if (!ec)
    {
        std::string command = message + "\n"; // Add newline for read_until
        boost::asio::write(socket, boost::asio::buffer(command), ec);
        if (ec)
        {
            std::cout << "Client write error: " << ec.message() << "\n";
            return false;
        }

        std::cout << "Client sent signal: " << message << "\n";
        return true;
    }
    else
    {
        std::cout << "Client connect error: " << ec.message() << "\n";
        return false;
    }
}
