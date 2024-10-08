#pragma once

#include <string>

#include <moduleWrapper.hpp>
#include <multitype_queue.hpp>

/// @brief Logcollector module class
class Logcollector {
public:
    void Start() const;
    void Setup(const configuration::ConfigurationParser& configurationParser);
    void Stop();
    Co_CommandExecutionResult ExecuteCommand(const std::string query);
    const std::string& Name() const { return m_moduleName; };
    void SetMessageQueue(const std::shared_ptr<IMultiTypeQueue> queue);

    static Logcollector& Instance()
    {
        static Logcollector s_instance;
        return s_instance;
    }

private:
    const std::string m_moduleName = "logcollector";
    bool m_enabled;
    std::shared_ptr<IMultiTypeQueue> m_messageQueue;
};
