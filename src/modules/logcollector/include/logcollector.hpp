#pragma once

#include <string>
#include <list>

#include <boost/asio/io_context.hpp>

#include <moduleWrapper.hpp>
#include <multitype_queue.hpp>

namespace logcollector {

class IReader;

/// @brief Logcollector module class
class Logcollector {
public:
    void Start();
    void Setup(const configuration::ConfigurationParser& configurationParser);
    void Stop();
    Co_CommandExecutionResult ExecuteCommand(const std::string query);
    const std::string& Name() const { return m_moduleName; };
    void SetMessageQueue(const std::shared_ptr<IMultiTypeQueue> queue);
    virtual void SendMessage(const std::string& location, const std::string& log);
    virtual void EnqueueTask(boost::asio::awaitable<void> task);
    virtual void AddReader(std::shared_ptr<IReader> reader);
    boost::asio::awaitable<void> Wait(std::chrono::milliseconds ms);
    boost::asio::awaitable<void> Wait(std::chrono::seconds sec);

    static Logcollector& Instance()
    {
        static Logcollector s_instance;
        return s_instance;
    }

protected:
    Logcollector() { }
    virtual ~Logcollector() = default;
    void SetupFileReader(const configuration::ConfigurationParser& configurationParser);

private:
    const std::string m_moduleName = "logcollector";
    bool m_enabled = true;
    std::shared_ptr<IMultiTypeQueue> m_messageQueue;
    boost::asio::io_context m_ioContext;
    std::list<std::shared_ptr<IReader>> m_readers;
};

}
