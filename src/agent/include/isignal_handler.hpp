#pragma once

/// @brief Interface for handling signals
///
/// Classes implementing this interface are responsible for properly handling
/// signals.
class ISignalHandler
{
public:
    /// @brief Virtual destructor
    virtual ~ISignalHandler() = default;

    /// @brief Waits for a signal to be received
    virtual void WaitForSignal() = 0;
};
