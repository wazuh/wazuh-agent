#include <iostream>

#ifdef ENABLE_INVENTORY
#include <inventory.hpp>
#endif

#ifdef ENABLE_LOGCOLLECTOR
#include <logcollector.hpp>
using logcollector::Logcollector;
#endif

#include <moduleManager.hpp>

void ModuleManager::AddModules() {

#ifdef ENABLE_INVENTORY
    AddModule(Inventory::Instance());
#endif

#ifdef ENABLE_LOGCOLLECTOR
    AddModule(Logcollector::Instance());
#endif

    Setup();
}

std::shared_ptr<ModuleWrapper> ModuleManager::GetModule(const std::string & name) {
    auto it = m_modules.find(name);
    if (it != m_modules.end()) {
        return it->second;
    }
    return nullptr;
}

void ModuleManager::Start() {
    for (const auto &[_, module] : m_modules) {
        m_createTask([module]() { module->Start(); });
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
}
