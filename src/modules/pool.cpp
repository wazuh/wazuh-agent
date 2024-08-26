#include <iostream>
#include "inventory.hpp"
#include "pool.hpp"

Pool::Pool() {
    // addModule(make_shared<LogCollector>());
    // addModule(make_shared<FIM>());
    addModule(Inventory::instance());
    // addModule(make_shared<SCA>());
}

template <typename T>
void Pool::addModule(T& module) {
    auto wrapper = make_shared<ModuleWrapper>(ModuleWrapper{
        .run = [&module]() { module.run(); },
        .setup = [&module](const Configuration & config) { return module.setup(config); },
        .stop = [&module]() { module.stop(); },
        .command = [&module](const string & query) { return module.command(query); },
        .name = [&module]() { return module.name(); }
    });

    modules[module.name()] = wrapper;
}

shared_ptr<ModuleWrapper> Pool::getModule(const string & name) {
    return modules.at(name);
}

void Pool::start() {
    for (const auto &[_, module] : modules) {
        threads.emplace_back([module]() { module->run(); });
    }
}

void Pool::setup(const Configuration & config) {
    for (const auto &[_, module] : modules) {
        module->setup(config);
    }
}

void Pool::stop() {
    for (const auto &[_, module] : modules) {
        module->stop();
    }

    for (auto &thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}