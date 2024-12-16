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

std::shared_ptr<ModuleWrapper> ModuleManager::GetModule(const std::string & name)
{
    if (auto it = m_modules.find(name); it != m_modules.end())
    {
        return it->second;
    }
    return nullptr;
}

void ModuleManager::Start()
{
    std::unique_lock<std::mutex> lock(m_mutex);

    m_taskManager.Start(m_modules.size());
    std::condition_variable cv;

    for (const auto &[_, module] : m_modules)
    {
        m_taskManager.EnqueueTask(
            [module, this, &cv]
            {
                ++m_started;
                cv.notify_one();
                module->Start();
            }
            , module->Name()
        );
    }

    cv.wait(
        lock,
        [this]
        {
            return m_started.load() == static_cast<int>(m_modules.size());
        }
    );
}

void ModuleManager::Setup()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto &[_, module] : m_modules)
    {
        module->Setup(m_configurationParser);
    }
}

void ModuleManager::Stop()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto &[_, module] : m_modules)
    {
        module->Stop();
        m_started--;
    }
    m_taskManager.Stop();
}
