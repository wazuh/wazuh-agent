#pragma once

#include <string>

#include <boost/asio/io_context.hpp>

#include <moduleWrapper.hpp>
#include <multitype_queue.hpp>

namespace logcollector {

/// @brief Logcollector module class
class Logcollector {
public:
    void Start();
    void Setup(const configuration::ConfigurationParser& configurationParser);
    void Stop();
    Co_CommandExecutionResult ExecuteCommand(const std::string query);
    const std::string& Name() const { return m_moduleName; };
    void SetMessageQueue(const std::shared_ptr<IMultiTypeQueue> queue);
    void SendMessage(const std::string& location, const std::string& log);
    void EnqueueTask(boost::asio::awaitable<void> task);

    static Logcollector& Instance()
    {
        static Logcollector s_instance;
        return s_instance;
    }

private:
    Logcollector() { }
    ~Logcollector() = default;

    const std::string m_moduleName = "logcollector";
    bool m_enabled;
    std::shared_ptr<IMultiTypeQueue> m_messageQueue;
    boost::asio::io_context m_ioContext;
};

}
