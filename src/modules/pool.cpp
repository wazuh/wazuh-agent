#include "pool.h"

Pool::Pool() {
    addModule(make_shared<Inventory>());
}

template <Module T>
void Pool::addModule(shared_ptr<T> module) {
    auto wrapper = make_shared<ModuleWrapper>(ModuleWrapper{
        .run = [module]() { module->run(); },
        .setup = [module](const Configuration & config) { return module->setup(config); },
        .stop = [module]() { module->stop(); },
        .command = [module](const string & query) { return module->command(query); }
    });

    modules[module->name()] = wrapper;
}

shared_ptr<ModuleWrapper> Pool::getModule(const string & name) {
    return modules.at(name);
}

void Pool::start() {
    for (const auto &[_, module] : modules) {
        module->run();
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
}

// Explicit template instantiation
template void Pool::addModule(shared_ptr<Inventory> module);
