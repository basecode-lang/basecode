// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

namespace basecode::common {

    template<typename T>
    struct remove_reference {
        typedef T Type;
    };

    template<typename T>
    struct remove_reference<T&> {
        typedef T Type;
    };

    template<typename T>
    struct remove_reference<T&&> {
        typedef T Type;
    };

    template<typename T>
    inline T&& forward(typename remove_reference<T>::Type& t) { return static_cast<T&&>(t); }

    template<typename T>
    inline T&& forward(typename remove_reference<T>::Type&& t) { return static_cast<T&&>(t); }

    template<typename T>
    inline T&& move(T&& t) { return static_cast<typename remove_reference<T>::Type&&>(t); }

    template<typename F>
    struct defer_wrapper {
        F f;

        defer_wrapper(F&& f) : f(forward<F>(f)) {}

        ~defer_wrapper() { f(); }
    };

    template<typename F>
    defer_wrapper<F> __defer_func(F&& f) { return defer_wrapper<F>(forward<F>(f)); }

    #define _DEFER_1(x, y) x##y
    #define _DEFER_2(x, y) _DEFER_1(x, y)
    #define _DEFER_3(x)    _DEFER_2(x, __COUNTER__)
    #define defer(code)    auto _DEFER_3(_defer_) = basecode::common::__defer_func([&]()->void{code;})

}