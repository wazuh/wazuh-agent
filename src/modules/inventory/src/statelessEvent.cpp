#include "statelessEvent.hpp"

constexpr int BYTES_IN_KILOBYTE = 1024;
constexpr int KILOBYTES_IN_MEGABYTE = 1024;
constexpr int BYTES_IN_MEGABYTE = BYTES_IN_KILOBYTE * KILOBYTES_IN_MEGABYTE;

StatelessEvent::StatelessEvent(std::string op, std::string time, nlohmann::json d)
    : operation(std::move(op)), created(std::move(time)), data(std::move(d)) {}

nlohmann::json NetworkEvent::generate() const {
    std::string interface = (data.contains("observer") &&
                         data["observer"].contains("ingress") &&
                         data["observer"]["ingress"].contains("interface") &&
                         data["observer"]["ingress"]["interface"].contains("name") &&
                         !data["observer"]["ingress"]["interface"]["name"].is_null())
                        ? data["observer"]["ingress"]["interface"]["name"].get<std::string>()
                        : "";

    std::string action, reason;

    if (operation == "create") {
        action = "network-interface-detected";
        reason = "New network interface " + interface + " detected";
    } else if (operation == "update") {
        action = "network-interface-updated";
        reason = "Network interface " + interface + " updated";
    } else {
        action = "network-interface-removed";
        reason = "Network interface " + interface + " was removed";
    }

    return {{"event", {{"action", action}, {"category", {"network"}}, {"type", {operation == "create" ? "info" : operation == "update" ? "change" : "deletion"}}, {"created", created}, {"reason", reason}}}};
}

nlohmann::json PackageEvent::generate() const {

    std::string packageName = (data.contains("package") && data["package"].contains("name") && !data["package"]["name"].is_null())
                              ? data["package"]["name"].get<std::string>() : "";
    std::string version = (data.contains("package") && data["package"].contains("version") && !data["package"]["version"].is_null())
                          ? data["package"]["version"].get<std::string>() : "";

    std::string action, reason;

    if (operation == "create") {
        action = "package-installed";
        reason = "Package " + packageName + " (version " + version + ") was installed";
    } else if (operation == "update") {
        action = "package-updated";
        reason = "Package " + packageName + " updated";
    } else {
        action = "package-removed";
        reason = "Package " + packageName + " (version " + version + ") was removed";
    }

    return {{"event", {{"action", action}, {"category", {"package"}}, {"type", {operation == "create" ? "installation" : operation == "update" ? "change" : "deletion"}}, {"created", created}, {"reason", reason}}}};
}

nlohmann::json HotfixEvent::generate() const {
    std::string hotfixID = (data.contains("package") && data["package"].contains("hotfix") && data["package"]["hotfix"].contains("name") && !data["package"]["hotfix"]["name"].is_null())
                       ? data["package"]["hotfix"]["name"].get<std::string>() : "";

    std::string action, reason;

    if (operation == "create") {
        action = "hotfix-installed";
        reason = "Hotfix " + hotfixID + " was installed";
    } else if (operation == "update") {
        action = "hotfix-updated";
        reason = "Hotfix " + hotfixID + " was updated";
    } else {
        action = "hotfix-removed";
        reason = "Hotfix " + hotfixID + " was removed";
    }

    return {{"event", {{"action", action}, {"category", {"hotfix"}}, {"type", {operation == "create" ? "installation" : "deletion"}}, {"created", created}, {"reason", reason}}}};
}

nlohmann::json PortEvent::generate() const {

    int srcPort = (data.contains("source") && data["source"].contains("port") && !data["source"]["port"].is_null())
                  ? data["source"]["port"].get<int>() : 0;
    int destPort = (data.contains("destination") && data["destination"].contains("port") && !data["destination"]["port"].is_null())
                   ? data["destination"]["port"].get<int>() : 0;

    std::string action, reason;

    if (operation == "create") {
        action = "port-detected";
        reason = "New connection from source port " + std::to_string(srcPort) + " to destination port " + std::to_string(destPort);
    } else if (operation == "update") {
        action = "port-updated";
        reason = "Updated connection from source port " + std::to_string(srcPort) + " to destination port " + std::to_string(destPort);
    } else {
        action = "port-closed";
        reason = "Closed connection from source port " + std::to_string(srcPort) + " to destination port " + std::to_string(destPort);
    }

    return {{"event", {{"action", action}, {"category", {"network"}}, {"type", {operation == "create" ? "connection" : operation == "update" ? "change" : "end"}}, {"created", created}, {"reason", reason}}}};
}

nlohmann::json ProcessEvent::generate() const {
    std::string processName = (data.contains("process") && data["process"].contains("name") && !data["process"]["name"].is_null())
                              ? data["process"]["name"].get<std::string>() : "";
    std::string pid = (data.contains("process") && data["process"].contains("pid") && !data["process"]["pid"].is_null())
                              ? data["process"]["name"].get<std::string>() : "";

    std::string action, reason;

    if (operation == "create") {
        action = "process-started";
        reason = "Process " + processName + " (PID: " + pid + ") was started";
    } else if (operation == "update") {
        action = "process-updated";
        reason = "Process " + processName + " (PID: " + pid + ") was updated";
    } else {
        action = "process-stopped";
        reason = "Process " + processName + " (PID: " + pid + ") was stopped";
    }

    return {{"event", {{"action", action}, {"category", {"process"}}, {"type", {operation == "create" ? "start" : operation == "update" ? "change" : "end"}}, {"created", created}, {"reason", reason}}}};
}

nlohmann::json SystemEvent::generate() const {

    std::string hostname = (data.contains("host") && data["host"].contains("hostname") && !data["host"]["hostname"].is_null())
                           ? data["host"]["hostname"].get<std::string>() : "";
    std::string osVersion = (data.contains("host") && data["host"].contains("os") && data["host"]["os"].contains("version") && !data["host"]["os"]["version"].is_null())
                            ? data["host"]["os"]["version"].get<std::string>() : "";

    std::string action = (operation == "update") ? "system-updated" : "system-detected";
    std::string reason = "System " + hostname + " is running OS version " + osVersion;

    return {{"event", {{"action", action}, {"category", {"host"}}, {"type", {operation == "update" ? "change" : "info"}}, {"created", created}, {"reason", reason}}}};
}

nlohmann::json HardwareEvent::generate() const {

    std::string cpuName = (data.contains("host") && data["host"].contains("cpu") && data["host"]["cpu"].contains("name") && !data["host"]["cpu"]["name"].is_null())
                          ? data["host"]["cpu"]["name"].get<std::string>() : "";
    int memoryTotalGB = (data.contains("host") && data["host"].contains("memory") && data["host"]["memory"].contains("total") && !data["host"]["memory"]["total"].is_null())
                        ? data["host"]["memory"]["total"].get<int>() / BYTES_IN_MEGABYTE : 0;
    std::string serialNumber = (data.contains("observer") && data["observer"].contains("serial_number") && !data["observer"]["serial_number"].is_null())
                               ? data["observer"]["serial_number"].get<std::string>() : "";

    std::string action, reason;

    if (operation == "create") {
        action = "hardware-detected";
        reason = "New hardware detected: " + cpuName + " with " + std::to_string(memoryTotalGB) + " GB memory";
    } else if (operation == "update") {
        action = "hardware-updated";
        reason = "Hardware changed";
    } else if (operation == "remove") {
        action = "hardware-removed";
        reason = "Hardware with serial number " + serialNumber + " was removed";
    }

    return {{"event", {{"action", action}, {"category", {"host"}}, {"type", {operation == "create" ? "start" : operation == "update" ? "change" : "removed"}}, {"created", created}, {"reason", reason}}}};
}

std::unique_ptr<StatelessEvent> CreateStatelessEvent(const std::string& type, const std::string& operation, const std::string& created, const nlohmann::json& data) {
    if (type == "networks") return std::make_unique<NetworkEvent>(operation, created, data);
    if (type == "packages") return std::make_unique<PackageEvent>(operation, created, data);
    if (type == "hotfixes") return std::make_unique<HotfixEvent>(operation, created, data);
    if (type == "ports") return std::make_unique<PortEvent>(operation, created, data);
    if (type == "processes") return std::make_unique<ProcessEvent>(operation, created, data);
    if (type == "system") return std::make_unique<SystemEvent>(operation, created, data);
    if (type == "hardware") return std::make_unique<HardwareEvent>(operation, created, data);

    return nullptr;
}
