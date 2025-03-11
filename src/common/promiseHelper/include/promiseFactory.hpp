#pragma once

#include "abstractWait.hpp"

namespace Utils
{
    /// @brief Promise type
    enum PromiseType
    {
        NORMAL,
        SLEEP
    };

    /// @brief Promise factory
    /// @tparam osType Promise type
    template<PromiseType osType>
    class PromiseFactory final
    {
    public:
        /// @brief Get promise
        /// @return promise
        static std::shared_ptr<IWait> getPromiseObject()
        {
            return std::make_shared<PromiseWaiting>();
        }
    };

    /// @brief Sleep promise
    template<>
    class PromiseFactory<PromiseType::SLEEP> final
    {
    public:
        /// @brief Get promise
        /// @return promise
        static std::shared_ptr<IWait> getPromiseObject()
        {
            return std::make_shared<BusyWaiting>();
        }
    };
} // namespace Utils
