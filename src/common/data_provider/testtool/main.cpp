#include "cmdLineActions.hpp"
#include "sysInfo.h"
#include "sysInfo.hpp"
#include <iostream>

constexpr auto JSON_PRETTY_SPACES {2};

class SysInfoPrinter final
{
public:
    SysInfoPrinter() = default;

    void PrintHardwareInfo()
    {
        m_data["hw"] = m_sysinfo.hardware();
    }

    void PrintNetworksInfo()
    {
        m_data["networks"] = m_sysinfo.networks();
    }

    void PrintOSInfo()
    {
        m_data["os"] = m_sysinfo.os();
    }

    void PrintPackagesInfo()
    {
        m_data["packages"] = m_sysinfo.packages();
    }

    void PrintProcessesInfo()
    {
        m_data["processes"] = m_sysinfo.processes();
    }

    void PrintPortsInfo()
    {
        m_data["ports"] = m_sysinfo.ports();
    }

    void PrintHotfixes()
    {
        m_data["hotfixes"] = m_sysinfo.hotfixes();
    }

    void PrintData()
    {
        std::cout << m_data.dump(JSON_PRETTY_SPACES) << '\n';
    }

    void PrintProcessesInfoCallback()
    {
        m_sysinfo.processes([this](nlohmann::json& process) { m_data["processes_cb"].push_back(process); });
    }

    void PrintPackagesInfoCallback()
    {
        m_sysinfo.packages([this](nlohmann::json& package) { m_data["packages_cb"].push_back(package); });
    }

private:
    SysInfo m_sysinfo;
    nlohmann::json m_data;
};

int main(int argc, const char* argv[])
{
    try
    {
        SysInfoPrinter printer;

        if (argc == 1)
        {
            // Calling testtool without parameters - default all
            printer.PrintHardwareInfo();
            printer.PrintNetworksInfo();
            printer.PrintOSInfo();
            printer.PrintPackagesInfo();
            printer.PrintProcessesInfo();
            printer.PrintPortsInfo();
            printer.PrintHotfixes();
            printer.PrintData();
            printer.PrintPackagesInfoCallback();
            printer.PrintProcessesInfoCallback();
        }
        else if (argc == 2)
        {
            const CmdLineActions cmdLineArgs(argv);

            if (cmdLineArgs.hardwareArg())
            {
                printer.PrintHardwareInfo();
            }
            else if (cmdLineArgs.networksArg())
            {
                printer.PrintNetworksInfo();
            }
            else if (cmdLineArgs.osArg())
            {
                printer.PrintOSInfo();
            }
            else if (cmdLineArgs.packagesArg())
            {
                printer.PrintPackagesInfo();
            }
            else if (cmdLineArgs.processesArg())
            {
                printer.PrintProcessesInfo();
            }
            else if (cmdLineArgs.portsArg())
            {
                printer.PrintPortsInfo();
            }
            else if (cmdLineArgs.hotfixesArg())
            {
                printer.PrintHotfixes();
            }
            else if (cmdLineArgs.packagesCallbackArg())
            {
                printer.PrintPackagesInfoCallback();
            }
            else if (cmdLineArgs.processesCallbackArg())
            {
                printer.PrintProcessesInfoCallback();
            }
            else
            {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                throw std::runtime_error {"Action value: " + std::string(argv[1]) + " not found."};
            }

            printer.PrintData();
        }
        else
        {
            throw std::runtime_error {"Multiple action are not allowed"};
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error getting system information: " << e.what() << '\n';
        CmdLineActions::showHelp();
    }
}
