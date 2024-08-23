#pragma once

class ISignalHandler
{
public:
    virtual ~ISignalHandler() = default;
    virtual void WaitForSignal() = 0;
};
