#pragma once

#define INVENTORY_DEFAULT_INTERVAL 3600

struct InventoryConfig {
    bool m_enabled = true;
    unsigned int m_interval = INVENTORY_DEFAULT_INTERVAL;
    bool m_scanOnStart = true;
    bool m_hardware = true;
    bool m_os = true;
    bool m_network = true;
    bool m_packages = true;
    bool m_ports = true;
    bool m_portsAll = true;
    bool m_processes = true;
    bool m_hotfixes = true;
};

class Configuration {
    public:
        Configuration() = default;

        Configuration(const InventoryConfig& config) : m_inventoryConfig(config) {}

        void SetInventoryConfig(const InventoryConfig& config) {
            m_inventoryConfig = config;
        }

        const InventoryConfig& GetInventoryConfig() const { return m_inventoryConfig; }

    private:
        InventoryConfig m_inventoryConfig;
};
