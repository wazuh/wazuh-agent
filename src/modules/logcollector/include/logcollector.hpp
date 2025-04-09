#pragma once

#include <command_entry.hpp>
#include <imodule.hpp>
#include <message.hpp>
#include <moduleWrapper.hpp>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>

#include <list>
#include <string>

namespace logcollector
{

    /// @brief Interface for log readers
    class IReader;

    /// @brief Logcollector module class
    ///
    /// This module is responsible for collecting logs from various sources and processing them.
    class Logcollector : public IModule
    {
    public:
        /// @copydoc IModule::Run
        void Run() override;

        /// @copydoc IModule::Setup
        void Setup(std::shared_ptr<const configuration::ConfigurationParser> configurationParser) override;

        /// @copydoc IModule::Stop
        void Stop() override;

        /// @copydoc IModule::ExecuteCommand
        Co_CommandExecutionResult ExecuteCommand(const std::string command, const nlohmann::json parameters) override;

        /// @copydoc IModule::Name
        const std::string& Name() const override
        {
            return m_moduleName;
        };

        /// @copydoc IModule::SetPushMessageFunction
        void SetPushMessageFunction(const std::function<int(Message)>& pushMessage) override;

        /// @brief Pushes a message to que queue
        /// @param location Location of the message
        /// @param log Message to send
        /// @param collectorType type of logcollector
        /// @pre The message queue must be set with SetMessageQueue
        virtual void PushMessage(const std::string& location, const std::string& log, const std::string& collectorType);

        /// @brief Enqueues an ASIO task (coroutine)
        /// @param task Task to enqueue
        virtual void EnqueueTask(boost::asio::awaitable<void> task);

        /// @brief Adds a reader
        /// @param reader Reader to add
        virtual void AddReader(std::shared_ptr<IReader> reader);

        /// @brief Waits for a specified amount of time
        ///
        /// @param ms Time to wait in milliseconds
        virtual boost::asio::awaitable<void> Wait(std::chrono::milliseconds ms);

        /// @brief Gets the instance of the Logcollector module
        /// @return Instance of the Logcollector module
        static Logcollector& Instance()
        {
            static Logcollector s_instance;
            return s_instance;
        }

        /// @brief Add platform specific implementation of IReader to logcollector.
        /// @param configurationParser where to get parameters.
        void AddPlatformSpecificReader(std::shared_ptr<const configuration::ConfigurationParser> configurationParser);

    protected:
        /// @brief Constructor
        Logcollector() {}

        /// @brief Destructor
        virtual ~Logcollector() = default;

        /// @brief Sets up the file reader
        /// @param configurationParser Configuration parser
        void SetupFileReader(const std::shared_ptr<const configuration::ConfigurationParser> configurationParser);

        /// @brief Clean all readers
        void CleanAllReaders();

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

        /// @brief Indicates if number of logs being monitorized
        std::atomic<int> m_activeReaders = 0;

        /// @brief Mutex to access steady timers list
        std::mutex m_timersMutex;

        /// @brief List of steady timers
        std::list<boost::asio::steady_timer*> m_timers;
    };

} // namespace logcollector
