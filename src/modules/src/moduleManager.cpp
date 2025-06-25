#include <moduleManager.hpp>

#ifdef ENABLE_INVENTORY
#include <inventory.hpp>
#endif

#ifdef ENABLE_LOGCOLLECTOR
#include <logcollector.hpp>
using logcollector::Logcollector;
#endif

#ifdef ENABLE_SCA
#include "sca_wrapper.hpp"
#endif

namespace
{
    constexpr int MODULES_START_WAIT_SECS = 60;
}

ModuleManager::ModuleManager(const std::function<int(Message)>& pushMessage,
                             std::shared_ptr<configuration::ConfigurationParser> configurationParser,
                             std::string uuid)
    : m_pushMessage(pushMessage)
    , m_configurationParser(std::move(configurationParser))
    , m_agentUUID(std::move(uuid))
{
    if (!m_pushMessage)
    {
        throw std::runtime_error("Invalid Push Message Function passed.");
    }

    if (!m_configurationParser)
    {
        throw std::runtime_error("Invalid Configuration Parser passed.");
    }
}

void ModuleManager::AddModules()
{
    {
        const std::lock_guard<std::mutex> lock(m_mutex);

#ifdef ENABLE_INVENTORY
        auto inventory = std::make_shared<Inventory>();
        inventory->SetAgentUUID(m_agentUUID);
        AddModule(inventory);
#endif

#ifdef ENABLE_LOGCOLLECTOR
        AddModule(std::make_shared<Logcollector>());
#endif

#ifdef ENABLE_SCA
        auto sca = std::make_shared<SCAWrapper>(m_configurationParser, m_agentUUID);
        AddModule(sca);
#endif
    }

    Setup();
}

void ModuleManager::AddModule(std::shared_ptr<IModule> module)
{
    const auto moduleName = module->Name();

    if (m_modules.find(moduleName) != m_modules.end())
    {
        throw std::runtime_error("Module with name '" + moduleName + "' already exists");
    }

    module->SetPushMessageFunction(m_pushMessage);
    m_modules[moduleName] = module;
}

std::shared_ptr<IModule> ModuleManager::GetModule(const std::string& name)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if (auto it = m_modules.find(name); it != m_modules.end())
    {
        return it->second;
    }
    return nullptr;
}

void ModuleManager::ReloadModule(const std::string& name)
{
    const std::lock_guard<std::mutex> lock(m_mutex);

    if (auto it = m_modules.find(name); it != m_modules.end())
    {
        it->second->Stop();

        --m_started;

        it->second->Setup(m_configurationParser);

        m_taskManager.EnqueueTask(
            [this, it]
            {
                ++m_started;
                it->second->Run();
            },
            it->second->Name());

        return;
    }

    LogError("Module {} not found", name);
}

void ModuleManager::Start()
{
    const std::lock_guard<std::mutex> lock(m_mutex);

    m_taskManager.StartThreadPool(m_modules.size());

    m_started.store(0);

    for (const auto& [_, module] : m_modules)
    {
        m_taskManager.EnqueueTask(
            [this, module]
            {
                ++m_started;
                module->Run();
            },
            module->Name());
    }

    const auto start = std::chrono::steady_clock::now();

    while (m_started.load() != static_cast<int>(m_modules.size()))
    {
        const auto end = std::chrono::steady_clock::now();
        const auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start);

        if (elapsed_seconds.count() > MODULES_START_WAIT_SECS)
        {
            break;
        }

        const auto sleepTime = std::chrono::milliseconds(10);
        std::this_thread::sleep_for(sleepTime);
    }

    if (m_started.load() != static_cast<int>(m_modules.size()))
    {
        LogError("Error when starting some modules. Modules started: {}", m_started.load());
    }
}

void ModuleManager::Setup()
{
    const std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto& [_, module] : m_modules)
    {
        module->Setup(m_configurationParser);
    }
}

void ModuleManager::Stop()
{
    const std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto& [_, module] : m_modules)
    {
        module->Stop();
    }
    m_taskManager.Stop();
}
