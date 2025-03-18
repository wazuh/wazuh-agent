#include "sysInfo.hpp"
#include "cjsonSmartDeleter.hpp"
#include "sysInfo.h"

nlohmann::json SysInfo::hardware()
{
    return getHardware();
}

nlohmann::json SysInfo::packages()
{
    return getPackages();
}

nlohmann::json SysInfo::os()
{
    return getOsInfo();
}

nlohmann::json SysInfo::processes()
{
    return getProcessesInfo();
}

nlohmann::json SysInfo::networks()
{
    return getNetworks();
}

nlohmann::json SysInfo::ports()
{
    return getPorts();
}

void SysInfo::processes(std::function<void(nlohmann::json&)> callback)
{
    getProcessesInfo(callback);
}

void SysInfo::packages(std::function<void(nlohmann::json&)> callback)
{
    getPackages(callback);
}

nlohmann::json SysInfo::hotfixes()
{
    return getHotfixes();
}

#ifdef __cplusplus
extern "C"
{
#endif
    int sysinfo_hardware(cJSON** jsResult)
    {
        auto retVal {-1};

        try
        {
            if (jsResult)
            {
                SysInfo info;
                const auto& hw {info.hardware()};
                *jsResult = cJSON_Parse(hw.dump().c_str());
                retVal = 0;
            }
        }
        catch (...) // NOLINT(bugprone-empty-catch)
        {
        }

        return retVal;
    }

    int sysinfo_packages(cJSON** jsResult)
    {
        auto retVal {-1};

        try
        {
            if (jsResult)
            {
                SysInfo info;
                const auto& packages {info.packages()};
                *jsResult = cJSON_Parse(packages.dump().c_str());
                retVal = 0;
            }
        }
        catch (...) // NOLINT(bugprone-empty-catch)
        {
        }

        return retVal;
    }

    int sysinfo_os(cJSON** jsResult)
    {
        auto retVal {-1};

        try
        {
            if (jsResult)
            {
                SysInfo info;
                const auto& os {info.os()};
                *jsResult = cJSON_Parse(os.dump().c_str());
                retVal = 0;
            }
        }
        catch (...) // NOLINT(bugprone-empty-catch)
        {
        }

        return retVal;
    }

    int sysinfo_processes(cJSON** jsResult)
    {
        auto retVal {-1};

        try
        {
            if (jsResult)
            {
                SysInfo info;
                const auto& processes {info.processes()};
                *jsResult = cJSON_Parse(processes.dump().c_str());
                retVal = 0;
            }
        }
        catch (...) // NOLINT(bugprone-empty-catch)
        {
        }

        return retVal;
    }

    int sysinfo_networks(cJSON** jsResult)
    {
        auto retVal {-1};

        try
        {
            if (jsResult)
            {
                SysInfo info;
                const auto& networks {info.networks()};
                *jsResult = cJSON_Parse(networks.dump().c_str());
                retVal = 0;
            }
        }
        catch (...) // NOLINT(bugprone-empty-catch)
        {
        }

        return retVal;
    }

    int sysinfo_ports(cJSON** jsResult)
    {
        auto retVal {-1};

        try
        {
            if (jsResult)
            {
                SysInfo info;
                const auto& ports {info.ports()};
                *jsResult = cJSON_Parse(ports.dump().c_str());
                retVal = 0;
            }
        }
        catch (...) // NOLINT(bugprone-empty-catch)
        {
        }

        return retVal;
    }

    void sysinfo_free_result(cJSON** jsData)
    {
        if (*jsData)
        {
            cJSON_Delete(*jsData);
        }
    }

    int sysinfo_packages_cb(callback_data_t callbackData)
    {
        auto retVal {-1};

        try
        {
            if (callbackData.callback)
            {
                const auto callbackWrapper {
                    [callbackData](nlohmann::json& jsonResult)
                    {
                        const std::unique_ptr<cJSON, CJsonSmartDeleter> spJson {cJSON_Parse(jsonResult.dump().c_str())};
                        callbackData.callback(GENERIC, spJson.get(), callbackData.user_data);
                    }};

                SysInfo info;
                info.packages(callbackWrapper);
                retVal = 0;
            }
        }
        catch (...) // NOLINT(bugprone-empty-catch)
        {
        }

        return retVal;
    }

    int sysinfo_processes_cb(callback_data_t callbackData)
    {
        auto retVal {-1};

        try
        {
            if (callbackData.callback)
            {
                const auto callbackWrapper {
                    [callbackData](nlohmann::json& jsonResult)
                    {
                        const std::unique_ptr<cJSON, CJsonSmartDeleter> spJson {cJSON_Parse(jsonResult.dump().c_str())};
                        callbackData.callback(GENERIC, spJson.get(), callbackData.user_data);
                    }};

                SysInfo info;
                info.processes(callbackWrapper);
                retVal = 0;
            }
        }
        catch (...) // NOLINT(bugprone-empty-catch)
        {
        }

        return retVal;
    }

    int sysinfo_hotfixes(cJSON** jsResult)
    {
        auto retVal {-1};

        try
        {
            if (jsResult)
            {
                SysInfo info;
                const auto& hotfixes {info.hotfixes()};
                *jsResult = cJSON_Parse(hotfixes.dump().c_str());
                retVal = 0;
            }
        }
        catch (...) // NOLINT(bugprone-empty-catch)
        {
        }

        return retVal;
    }

#ifdef __cplusplus
}
#endif
