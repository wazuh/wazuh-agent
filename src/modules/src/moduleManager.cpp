#include <iostream>
#include "inventory.h"
#include "moduleManager.h"

ModuleManager::ModuleManager() {
    addModule(Inventory::instance());
}

template <typename T>
void ModuleManager::addModule(T& module) {
    auto wrapper = make_shared<ModuleWrapper>(ModuleWrapper{
        .start = [&module]() { module.start(); },
        .setup = [&module](const Configuration & config) { return module.setup(config); },
        .stop = [&module]() { module.stop(); },
        .command = [&module](const string & query) { return module.command(query); },
        .name = [&module]() { return module.name(); }
    });

    modules[module.name()] = wrapper;
}

shared_ptr<ModuleWrapper> ModuleManager::getModule(const string & name) {
    return modules.at(name);
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