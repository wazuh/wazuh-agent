#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class OSLogStoreWrapper
{
public:
    class Iterator
    {
    public:
        friend class OSLogStoreWrapper;

        using IteratorCategory = std::input_iterator_tag;
        using ValueType = std::string;
        using DifferenceType = std::ptrdiff_t;
        using Pointer = const ValueType*;
        using Reference = const ValueType&;

        Reference operator*() const;
        Pointer operator->() const;
        Iterator& operator++();
        bool operator==(const Iterator& other) const;
        bool operator!=(const Iterator& other) const;

        ~Iterator();

    private:
        /// @brief Default constructor for the iterator.
        /// Represents the end iterator.
        Iterator() = default;

        /// @brief Constructs an iterator with the given OSLogEnumerator.
        /// @param wrapper The parent OSLogStoreWrapper instance.
        /// @param osLogEnumerator The OSLogEnumerator instance for iteration.
        Iterator(OSLogStoreWrapper* wrapper, void* osLogEnumerator);

        /**
         * @brief Advances the enumerator to the next log entry and updates the current log.
         */
        void Advance();

        OSLogStoreWrapper* m_wrapper = nullptr;
        void* m_osLogEnumerator = nullptr;
        std::string m_currentLog;
    };

    OSLogStoreWrapper();
    ~OSLogStoreWrapper();

    Iterator Begin(const char* startTimeStr = nullptr, const char* endTimeStr = nullptr);
    Iterator End();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};
