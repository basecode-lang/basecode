// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//       C O M P I L E R  P R O J E C T
//
// Copyright (C) 2019 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

namespace basecode {

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

}

#define _DEFER_1(x, y) x##y
#define _DEFER_2(x, y) _DEFER_1(x, y)
#define _DEFER_3(x)    _DEFER_2(x, __COUNTER__)
#define defer(code)    auto _DEFER_3(_defer_) = basecode::__defer_func([&]()->void{code;})
