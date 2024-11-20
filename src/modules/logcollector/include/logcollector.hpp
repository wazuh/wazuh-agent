#pragma once

#include <string>
#include <list>

#include <boost/asio/io_context.hpp>

#include <moduleWrapper.hpp>
#include <multitype_queue.hpp>

namespace logcollector {

/// @brief Interface for log readers
class IReader;

/// @brief Logcollector module class
///
/// This module is responsible for collecting logs from various sources and processing them.
class Logcollector {
public:
    /// @brief Starts the module
    void Start();

    /// @brief Configures the module
    /// @param configurationParser Configuration parser
    void Setup(const configuration::ConfigurationParser& configurationParser);

    /// @brief Stops the module
    void Stop();

    /// @brief Executes a command
    /// @param command Command to execute
    /// @param parameters A json object containing the parameters of the command to be executed
    /// @return Awaitable (coroutine) which will return the result of the command execution
    Co_CommandExecutionResult ExecuteCommand(const std::string command, const nlohmann::json parameters);

    /// @brief Gets the name of the Logcollector module
    /// @return Name of the module
    const std::string& Name() const { return m_moduleName; };

    /// @brief Sets the push message function
    /// @param pushMessage Push message function
    void SetPushMessageFunction(const std::function<int(Message)>& pushMessage);

    /// @brief Sends a message to que queue
    /// @param location Location of the message
    /// @param log Message to send
    /// @param collectorType type of logcollector
    /// @pre The message queue must be set with SetMessageQueue
    virtual void SendMessage(const std::string& location, const std::string& log, const std::string& collectorType);

    /// @brief Enqueues an ASIO task (coroutine)
    /// @param task Task to enqueue
    virtual void EnqueueTask(boost::asio::awaitable<void> task);

    /// @brief Adds a reader
    /// @param reader Reader to add
    virtual void AddReader(std::shared_ptr<IReader> reader);

    /// @brief Waits for a specified amount of time
    ///
    /// @param ms Time to wait in milliseconds
    boost::asio::awaitable<void> Wait(std::chrono::milliseconds ms);

    /// @brief Gets the instance of the Logcollector module
    /// @return Instance of the Logcollector module
    static Logcollector& Instance()
    {
        static Logcollector s_instance;
        return s_instance;
    }

protected:
    /// @brief Constructor
    Logcollector() { }

    /// @brief Destructor
    virtual ~Logcollector() = default;

    /// @brief Sets up the file reader
    /// @param configurationParser Configuration parser
    void SetupFileReader(const configuration::ConfigurationParser& configurationParser);

private:
    /// @brief Module name
    const std::string m_moduleName = "logcollector";

    /// @brief Is the module enabled
    bool m_enabled = true;

    /// @brief Push message function
    std::function<int(Message)> m_pushMessage;

    /// @brief Boost ASIO context
    boost::asio::io_context m_ioContext;

    /// @brief List of readers
    std::list<std::shared_ptr<IReader>> m_readers;
};

}
