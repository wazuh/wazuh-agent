#include <gtest/gtest.h>
#include "../src/statelessEvent.hpp"

TEST(PackageEventTest, CreatePackageEvent) {
    nlohmann::json data = {
        {"package", {{"name", "nginx"}, {"version", "1.18.0"}}}
    };

    PackageEvent event("create", "2024-01-16T00:00:00Z", data);
    nlohmann::json result = event.generate();

    EXPECT_EQ(result["event"]["action"], "package-installed");
    EXPECT_EQ(result["event"]["category"][0], "package");
    EXPECT_EQ(result["event"]["reason"], "Package nginx (version 1.18.0) was installed");
}

TEST(NetworkEventTest, CreateNetworkEvent) {
    nlohmann::json data = {
        {"observer", {{"ingress", {{"interface", {{"name", "eth0"}}}}}}}
    };

    NetworkEvent event("create", "2024-01-16T00:00:00Z", data);
    nlohmann::json result = event.generate();

    EXPECT_EQ(result["event"]["action"], "network-interface-detected");
    EXPECT_EQ(result["event"]["category"][0], "network");
    EXPECT_EQ(result["event"]["reason"], "New network interface eth0 detected");
}

TEST(HotfixEventTest, RemoveHotfixEvent) {
    nlohmann::json data = {
        {"package", {{"hotfix", {{"name", "KB123456"}}}}}
    };

    HotfixEvent event("remove", "2024-01-16T00:00:00Z", data);
    nlohmann::json result = event.generate();

    EXPECT_EQ(result["event"]["action"], "hotfix-removed");
    EXPECT_EQ(result["event"]["category"][0], "hotfix");
    EXPECT_EQ(result["event"]["reason"], "Hotfix KB123456 was removed");
}

TEST(PortEventTest, UpdatePortEvent) {
    nlohmann::json data = nlohmann::json::parse(R"(
        {
            "source": { "port": 8080 },
            "destination": { "port": 443 }
        }
    )");

    PortEvent event("update", "2024-01-16T00:00:00Z", data);
    nlohmann::json result = event.generate();

    EXPECT_EQ(result["event"]["action"], "port-updated");
    EXPECT_EQ(result["event"]["category"][0], "network");
    EXPECT_EQ(result["event"]["type"][0], "change");
    EXPECT_EQ(result["event"]["reason"], "Updated connection from source port 8080 to destination port 443");
}

TEST(SystemEventTest, CreateSystemEvent) {
    nlohmann::json data = {
        {"host", {{"hostname", "server01"}, {"os", {{"version", "Ubuntu 22.04"}}}}}
    };

    SystemEvent event("create", "2024-01-16T00:00:00Z", data);
    nlohmann::json result = event.generate();

    EXPECT_EQ(result["event"]["action"], "system-detected");
    EXPECT_EQ(result["event"]["category"][0], "host");
    EXPECT_EQ(result["event"]["type"][0], "info");
    EXPECT_EQ(result["event"]["reason"], "System server01 is running OS version Ubuntu 22.04");
    EXPECT_EQ(result["event"]["created"], "2024-01-16T00:00:00Z");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
