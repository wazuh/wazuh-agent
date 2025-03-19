#pragma once

#include "commonDefs.h"

#include <cjson/cJSON.h>
#ifdef __cplusplus
extern "C"
{
#endif

    /// @brief Obtains the hardware information from the current OS being analyzed.
    /// @param jsResult Resulting json where the specific information will be stored.
    /// @return 0 on success, -1 otherwise.
    int sysinfo_hardware(cJSON** jsResult);

    /// @brief Obtains the installed packages information from the current OS being analyzed.
    /// @param jsResult Resulting json where the specific information will be stored.
    /// @return 0 on success, -1 otherwise.
    int sysinfo_packages(cJSON** jsResult);

    /// @brief Obtains the Operating System information from the current OS being analyzed.
    /// @param jsResult Resulting json where the specific information will be stored.
    /// @return 0 on success, -1 otherwise.
    int sysinfo_os(cJSON** jsResult);

    /// @brief Obtains the processes information from the current OS being analyzed.
    /// @param jsResult Resulting json where the specific information will be stored.
    /// @return 0 on success, -1 otherwise.
    int sysinfo_processes(cJSON** jsResult);

    /// @brief Obtains the network information from the current OS being analyzed.
    /// @param jsResult Resulting json where the specific information will be stored.
    /// @return 0 on success, -1 otherwise.
    int sysinfo_networks(cJSON** jsResult);

    /// @brief Obtains the ports information from the current OS being analyzed.
    /// @param jsResult Resulting json where the specific information will be stored.
    /// @return 0 on success, -1 otherwise.
    int sysinfo_ports(cJSON** jsResult);

    /// @brief Frees the \p jsData information.
    /// @param jsData Information to be freed.
    void sysinfo_free_result(cJSON** jsData);

    /// @brief Obtains the processes information from the current OS being analyzed.
    /// @param callbackData Resulting single process data where the specific information will be stored.
    /// return 0 on success, -1 otherwhise.
    int sysinfo_processes_cb(callback_data_t callbackData);

    /// @brief Obtains the packages information from the current OS being analyzed.
    /// @param callbackData Resulting single package data where the specific information will be stored.
    /// return 0 on success, -1 otherwhise.
    int sysinfo_packages_cb(callback_data_t callbackData);

    /// @brief Obtains the hotfixes information from the current OS being analyzed.
    /// @param jsResult Resulting json where the specific information will be stored.
    /// @return 0 on success, -1 otherwise.
    int sysinfo_hotfixes(cJSON** jsResult);

    typedef int (*sysinfo_networks_func)(cJSON** jsresult);
    typedef int (*sysinfo_os_func)(cJSON** jsresult);
    typedef int (*sysinfo_processes_func)(cJSON** jsresult);
    typedef void (*sysinfo_free_result_func)(cJSON** jsresult);

#ifdef __cplusplus
}
#endif
