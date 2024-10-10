#include <iostream>

#ifdef ENABLE_MODULE_INVENTORY
#include <inventory.hpp>
#endif

#include <moduleManager.hpp>

std::shared_ptr<ModuleWrapper> ModuleManager::GetModule(const std::string & name) {
    auto it = m_modules.find(name);
    if (it != m_modules.end()) {
        return it->second;
    }
    return nullptr;
}

void ModuleManager::Start() {
    for (const auto &[_, module] : m_modules) {
        m_threads.emplace_back([module]() { module->Start(); });
    }
}

void ModuleManager::Setup() {
    for (const auto &[_, module] : m_modules) {
        module->Setup(m_configurationParser);
    }
}

void ModuleManager::Stop() {
    for (const auto &[_, module] : m_modules) {
        module->Stop();
    }

    for (auto &thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void ModuleManager::Init() {

#ifdef ENABLE_MODULE_INVENTORY
    AddModule(Inventory::Instance());
#endif

    Setup();

    Start();

}