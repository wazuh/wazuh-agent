#include <moduleManager.hpp>

#ifdef ENABLE_INVENTORY
#include <inventory.hpp>
#endif

#ifdef ENABLE_LOGCOLLECTOR
#include <logcollector.hpp>
using logcollector::Logcollector;
#endif

void ModuleManager::AddModules() {

#ifdef ENABLE_INVENTORY
    Inventory& inventory = Inventory::Instance();
    inventory.SetAgentUUID(m_agentUUID);
    AddModule(inventory);
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
    m_taskManager.Start(m_modules.size());
    for (const auto &[_, module] : m_modules)
    {
        m_taskManager.EnqueueTask([module]() { module->Start(); }, module->Name());
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
    m_taskManager.Stop();
}
