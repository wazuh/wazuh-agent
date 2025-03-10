#pragma once

template<typename F, F func>
struct CustomDeleter
{
    template<typename T>
    constexpr void operator()(T* arg) const
    {
        func(arg);
    }
};
