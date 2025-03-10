#pragma once

#include <map>
#include <mutex>

namespace Utils
{
    /// @brief Safe map
    template<typename Key, typename Value>
    class MapWrapperSafe final
    {
        std::map<Key, Value> m_map;
        std::mutex m_mutex;

    public:
        /// @brief Default constructor
        MapWrapperSafe() = default;

        /// @brief Insert a key-value pair
        /// @param key The key
        /// @param value The value
        void insert(const Key& key, const Value& value)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_map.emplace(key, value);
        }

        /// @brief Get the value associated with the key
        /// @param key The key
        /// @return The value associated with the key
        Value operator[](const Key& key)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            const auto it {m_map.find(key)};
            return m_map.end() != it ? it->second : Value();
        }

        /// @brief Erase the key-value pair
        /// @param key The key
        void erase(const Key& key)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_map.erase(key);
        }
    };
} // namespace Utils
