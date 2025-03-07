#pragma once

#include <isignal_handler.hpp>

#include <atomic>
#include <condition_variable>
#include <csignal>
#include <mutex>
#include <vector>

/// @brief Class for handling signals
class SignalHandler : public ISignalHandler
{
public:
    /// @brief Constructor
    /// @param signalsToHandle The signals to be handled by this class
    SignalHandler(const std::vector<int>& signalsToHandle = {SIGINT, SIGTERM});

    /// @copydoc ISignalHandler::WaitForSignal
    void WaitForSignal() override;

    /// @brief Handles the given signal
    /// @param signalToHandle The signal to be handled
    static void HandleSignal(int signalToHandle);

    /// @brief Keeps track of whether the agent should continue running
    static std::atomic<bool> KeepRunning;

private:
    /// @brief Initializes signal handling for the specified signals
    /// @param signalsToHandle The signals that this handler should manage
    void Initialize(const std::vector<int>& signalsToHandle);

    /// @brief Condition variable for synchronizing signal handling
    static std::condition_variable m_cv;

    /// @brief Mutex for protecting condition variable access
    static std::mutex m_cvMutex;
};
