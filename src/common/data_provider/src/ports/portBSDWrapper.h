#pragma once

#include <netdb.h>
#include <sys/proc_info.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "iportWrapper.h"
#include "sharedDefs.h"
#include "stringHelper.hpp"

static const std::map<int32_t, std::string> PORTS_TYPE = {{SOCKINFO_TCP, "tcp"}, {SOCKINFO_IN, "udp"}};

static const std::map<int32_t, std::string> STATE_TYPE = {{TSI_S_ESTABLISHED, "established"},
                                                          {TSI_S_SYN_SENT, "syn_sent"},
                                                          {TSI_S_SYN_RECEIVED, "syn_recv"},
                                                          {TSI_S_FIN_WAIT_1, "fin_wait1"},
                                                          {TSI_S_FIN_WAIT_2, "fin_wait2"},
                                                          {TSI_S_TIME_WAIT, "time_wait"},
                                                          {TSI_S_CLOSED, "close"},
                                                          {TSI_S__CLOSE_WAIT, "close_wait"},
                                                          {TSI_S_LAST_ACK, "last_ack"},
                                                          {TSI_S_LISTEN, "listening"},
                                                          {TSI_S_CLOSING, "closing"}};

/// @brief BSD process information
struct ProcessInfo
{
    int32_t pid;
    std::string processName;

    bool operator<(const ProcessInfo& src) const
    {
        return this->pid < src.pid;
    }
};

/// @brief BSD port wrapper
class BSDPortWrapper final : public IPortWrapper
{
    ProcessInfo m_processInformation;
    std::shared_ptr<socket_fdinfo> m_spSocketInfo;

public:
    /// @brief Constructor
    /// @param processInformation process information
    /// @param socketInfo socket information
    explicit BSDPortWrapper(const ProcessInfo& processInformation, const std::shared_ptr<socket_fdinfo>& socketInfo)
        : m_processInformation {processInformation}
        , m_spSocketInfo {socketInfo}
    {
        if (!m_spSocketInfo)
        {
            throw std::runtime_error {"Invalid socket FD information"};
        }
    };

    /// @brief Destructor
    ~BSDPortWrapper() = default;

    /// @copydoc IPortWrapper::protocol
    void protocol(nlohmann::json& port) const override
    {
        port["protocol"] = EMPTY_VALUE;
        const auto it {PORTS_TYPE.find(m_spSocketInfo->psi.soi_kind)};

        if (it != PORTS_TYPE.end())
        {
            port["protocol"] = AF_INET6 == m_spSocketInfo->psi.soi_family ? it->second + "6" : it->second;
        }
    }

    /// @copydoc IPortWrapper::localIp
    void localIp(nlohmann::json& port) const override
    {
        char ipAddress[NI_MAXHOST] {0};

        if (AF_INET6 == m_spSocketInfo->psi.soi_family)
        {
            sockaddr_in6 socketAddressIn6 {};
            socketAddressIn6.sin6_family = m_spSocketInfo->psi.soi_family;
            socketAddressIn6.sin6_addr = static_cast<in6_addr>(m_spSocketInfo->psi.soi_proto.pri_in.insi_laddr.ina_6);
            getnameinfo(reinterpret_cast<sockaddr*>(&socketAddressIn6),
                        sizeof(socketAddressIn6),
                        ipAddress,
                        sizeof(ipAddress),
                        nullptr,
                        0,
                        NI_NUMERICHOST);
        }
        else if (AF_INET == m_spSocketInfo->psi.soi_family)
        {
            sockaddr_in socketAddressIn {};
            socketAddressIn.sin_family = m_spSocketInfo->psi.soi_family;
            socketAddressIn.sin_addr =
                static_cast<in_addr>(m_spSocketInfo->psi.soi_proto.pri_in.insi_laddr.ina_46.i46a_addr4);
            getnameinfo(reinterpret_cast<sockaddr*>(&socketAddressIn),
                        sizeof(socketAddressIn),
                        ipAddress,
                        sizeof(ipAddress),
                        nullptr,
                        0,
                        NI_NUMERICHOST);
        }

        port["local_ip"] = Utils::substrOnFirstOccurrence(ipAddress, "%");
    }

    /// @copydoc IPortWrapper::localPort
    void localPort(nlohmann::json& port) const override
    {
        port["local_port"] = ntohs(m_spSocketInfo->psi.soi_proto.pri_in.insi_lport);
    }

    /// @copydoc IPortWrapper::remoteIP
    void remoteIP(nlohmann::json& port) const override
    {
        char ipAddress[NI_MAXHOST] {0};

        if (AF_INET6 == m_spSocketInfo->psi.soi_family)

        {
            sockaddr_in6 socketAddressIn6 {};
            socketAddressIn6.sin6_family = m_spSocketInfo->psi.soi_family;
            socketAddressIn6.sin6_addr = static_cast<in6_addr>(m_spSocketInfo->psi.soi_proto.pri_in.insi_faddr.ina_6);
            getnameinfo(reinterpret_cast<sockaddr*>(&socketAddressIn6),
                        sizeof(socketAddressIn6),
                        ipAddress,
                        sizeof(ipAddress),
                        nullptr,
                        0,
                        NI_NUMERICHOST);
        }
        else if (AF_INET == m_spSocketInfo->psi.soi_family)
        {
            sockaddr_in socketAddressIn {};
            socketAddressIn.sin_family = m_spSocketInfo->psi.soi_family;
            socketAddressIn.sin_addr =
                static_cast<in_addr>(m_spSocketInfo->psi.soi_proto.pri_in.insi_faddr.ina_46.i46a_addr4);
            getnameinfo(reinterpret_cast<sockaddr*>(&socketAddressIn),
                        sizeof(socketAddressIn),
                        ipAddress,
                        sizeof(ipAddress),
                        nullptr,
                        0,
                        NI_NUMERICHOST);
        }

        port["remote_ip"] = Utils::substrOnFirstOccurrence(ipAddress, "%");
    }

    /// @copydoc IPortWrapper::remotePort
    void remotePort(nlohmann::json& port) const override
    {
        port["remote_port"] = ntohs(m_spSocketInfo->psi.soi_proto.pri_in.insi_fport);
    }

    /// @copydoc IPortWrapper::txQueue
    void txQueue(nlohmann::json& port) const override
    {
        port["tx_queue"] = UNKNOWN_VALUE;
    }

    /// @copydoc IPortWrapper::rxQueue
    void rxQueue(nlohmann::json& port) const override
    {
        port["rx_queue"] = UNKNOWN_VALUE;
    }

    /// @copydoc IPortWrapper::inode
    void inode(nlohmann::json& port) const override
    {
        port["inode"] = 0;
    }

    /// @copydoc IPortWrapper::state
    void state(nlohmann::json& port) const override
    {
        port["state"] = UNKNOWN_VALUE;

        const auto itProtocol {PORTS_TYPE.find(m_spSocketInfo->psi.soi_kind)};

        if (PORTS_TYPE.end() != itProtocol && SOCKINFO_TCP == itProtocol->first)
        {
            const auto itState {STATE_TYPE.find(m_spSocketInfo->psi.soi_proto.pri_tcp.tcpsi_state)};

            if (itState != STATE_TYPE.end())
            {
                port["state"] = itState->second;
            }
        }
    }

    /// @copydoc IPortWrapper::pid
    void pid(nlohmann::json& port) const override
    {
        port["pid"] = m_processInformation.pid;
    }

    /// @copydoc IPortWrapper::processName
    void processName(nlohmann::json& port) const override
    {
        port["process"] = m_processInformation.processName;
    }
};
