#pragma once

#include <mutex>
#include <shared_mutex>

/// @brief Abstract class for locking
class ILocking
{
public:
    /// @brief Default destructor
    virtual ~ILocking() = default;

    /// @brief Locks the mutex
    virtual void lock() = 0;

    /// @brief Unlocks the mutex
    virtual void unlock() = 0;
};

/// @brief Shared locking
class SharedLocking final : public ILocking
{
    std::shared_lock<std::shared_timed_mutex> m_lock;

public:
    /// @brief Constructor
    /// @param mutex Mutex
    explicit SharedLocking(std::shared_timed_mutex& mutex)
        : m_lock(std::shared_lock<std::shared_timed_mutex>(mutex)) {};

    /// @brief Default destructor
    virtual ~SharedLocking() = default;

    /// @copydoc ILocking::lock
    virtual void lock() override
    {
        m_lock.lock();
    }

    /// @copydoc ILocking::unlock
    virtual void unlock() override
    {
        m_lock.unlock();
    }
};

/// @brief Exclusive locking
class ExclusiveLocking final : public ILocking
{
    std::unique_lock<std::shared_timed_mutex> m_lock;

public:
    /// @brief Constructor
    /// @param mutex Mutex
    explicit ExclusiveLocking(std::shared_timed_mutex& mutex)
        : m_lock(std::unique_lock<std::shared_timed_mutex>(mutex)) {};

    /// @brief Default destructor
    virtual ~ExclusiveLocking() = default;

    /// @copydoc ILocking::lock
    virtual void lock() override
    {
        m_lock.lock();
    }

    /// @copydoc ILocking::unlock
    virtual void unlock() override
    {
        m_lock.unlock();
    }
};
