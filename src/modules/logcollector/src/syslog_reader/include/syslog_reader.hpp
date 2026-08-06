#pragma once

#include <reader.hpp>

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

/// @brief Collector type reported for messages received by an agent-side syslog listener
const std::string REMOTE_SYSLOG_READER_TYPE = "remote-syslog";

// ----------------------------------------------------------------------------
// Future work (tracked against wazuh/wazuh#15178)
//
// This reader covers the UDP/TCP IP-socket ingress only. The following remain
// to be implemented as follow-ups and are referenced here for convenience:
//
//   1. UNIX domain socket listeners (unix_stream / unix_dgram / unix_seq),
//      using boost::asio::local::stream_protocol / datagram_protocol. See the
//      manager-side Fluentd forwarder and agent instance_communicator for the
//      existing local-socket pattern.
//   2. Named pipe / FIFO ingress (Linux mkfifo; Windows named pipes use a
//      different API). Equivalent to the legacy "syslog-pipe" log format.
//   3. TLS for the TCP listener (boost::asio::ssl::stream), with cert/key/CA
//      configuration and optional client-certificate verification.
//   4. TCP octet-counting framing (RFC 6587): currently only newline-delimited
//      ("non-transparent") framing is parsed; detect a leading "<digits> " to
//      support octet-counted messages.
//   5. Hostname bind_address resolution (currently numeric IP literals only).
//   6. allowed-ips source filtering (currently rely on host firewall rules).
// ----------------------------------------------------------------------------

namespace logcollector
{
    /// @brief Transport protocol used by an agent-side syslog listener
    enum class SyslogProtocol
    {
        Udp,
        Tcp
    };

    /// @brief Agent-side syslog listener reader
    ///
    /// This reader opens a UDP or TCP socket and listens for incoming syslog
    /// messages. Each received message is forwarded into the normal Logcollector
    /// processing path so that it is sent through the standard agent-to-manager
    /// pipeline and remains associated with the receiving agent.
    ///
    /// A single reader handles one listener definition (one protocol, bind address
    /// and port). Multiple listeners are represented by multiple reader instances.
    class SyslogReader : public IReader
    {
    public:
        /// @brief Constructor
        /// @param pushMessageFunc Push message function
        /// @param waitFunc Wait function
        /// @param enqueueTaskFunc Enqueue task function (used to spawn TCP client handlers)
        /// @param protocol Listener transport protocol (UDP or TCP)
        /// @param bindAddress Address the listener binds to
        /// @param port Port the listener binds to (1-65535)
        SyslogReader(
            std::function<void(const std::string& location, const std::string& log, const std::string& collectorType)>
                pushMessageFunc,
            std::function<Awaitable(std::chrono::milliseconds)> waitFunc,
            std::function<void(boost::asio::awaitable<void>)> enqueueTaskFunc,
            SyslogProtocol protocol,
            std::string bindAddress,
            std::uint16_t port);

        /// @copydoc IReader::Run
        Awaitable Run() override;

        /// @copydoc IReader::Stop
        void Stop() override;

        /// @brief Returns a human-readable identifier for this listener
        /// @return Identifier in the form "<protocol>:<bind_address>:<port>"
        std::string ListenerId() const;

        /// @brief Converts a protocol enum value to its lowercase string representation
        /// @param protocol Protocol to convert
        /// @return "udp" or "tcp"
        static std::string ProtocolToString(SyslogProtocol protocol);

        /// @brief Maximum size in bytes of a single syslog message accepted by a listener
        static constexpr std::size_t MAX_MESSAGE_SIZE = 65536;

    private:
        /// @brief Runs the UDP listener loop
        Awaitable RunUdp();

        /// @brief Runs the TCP accept loop
        Awaitable RunTcp();

        /// @brief Handles a single accepted TCP client connection
        /// @param socket Connected client socket
        /// @param remote Human-readable remote endpoint description
        Awaitable HandleTcpClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::string remote);

        /// @brief Normalizes and forwards a received message into the Logcollector path
        /// @param message Raw message received from the network
        void ProcessMessage(std::string message) const;

        /// @brief Enqueue task function
        std::function<void(boost::asio::awaitable<void>)> m_enqueueTask;

        /// @brief Listener transport protocol
        SyslogProtocol m_protocol;

        /// @brief Bind address
        std::string m_bindAddress;

        /// @brief Bind port
        std::uint16_t m_port;

        /// @brief Collector type reported to the Logcollector pipeline
        const std::string m_collectorType = REMOTE_SYSLOG_READER_TYPE;

        /// @brief Protects the executor and socket handles shared with Stop()
        std::mutex m_socketMutex;

        /// @brief Executor of the io_context the reader runs on (set when Run starts)
        std::optional<boost::asio::any_io_executor> m_executor;

        /// @brief UDP socket (when protocol is UDP)
        std::shared_ptr<boost::asio::ip::udp::socket> m_udpSocket;

        /// @brief TCP acceptor (when protocol is TCP)
        std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;

        /// @brief Active TCP client sockets, tracked so they can be closed on shutdown
        std::list<std::weak_ptr<boost::asio::ip::tcp::socket>> m_clients;
    };

} // namespace logcollector
