#include <iostream>
#include "inventory.hpp"
#include "moduleManager.hpp"

shared_ptr<ModuleWrapper> ModuleManager::getModule(const string & name) {
    auto it = modules.find(name);
    if (it != modules.end()) {
        return it->second;
    }
    return nullptr;
}

void ModuleManager::start() {
    for (const auto &[_, module] : modules) {
        threads.emplace_back([module]() { module->start(); });
    }
}

void ModuleManager::setup(const Configuration & config) {
    for (const auto &[_, module] : modules) {
        module->setup(config);
    }
}

void ModuleManager::stop() {
    for (const auto &[_, module] : modules) {
        module->stop();
    }

    for (auto &thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}
