#pragma once

#include "stringHelper.h"

#define INVENTORY_DEFAULT_INTERVAL W_HOUR_SECONDS

struct InventoryConfig {
    bool enabled = true;
    unsigned int interval = INVENTORY_DEFAULT_INTERVAL;
    bool scanOnStart = true;
    bool hardware = true;
    bool os = true;
    bool network = true;
    bool packages = true;
    bool ports = true;
    bool portsAll = true;
    bool processes = true;
    bool hotfixes = true;
};

class Configuration {
    public:
        Configuration() = default;

        Configuration(const InventoryConfig& config) : inventoryConfig(config) {}

        void setInventoryConfig(const InventoryConfig& config) {
            inventoryConfig = config;
        }

        const InventoryConfig& getInventoryConfig() const { return inventoryConfig; }

    private:
        InventoryConfig inventoryConfig;
};
