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

#include <basecode/context/context.h>
#include "allocator.h"

namespace basecode::memory {

    template<class T>
    class std_allocator_t {
    public:
        using value_type = T;

        explicit std_allocator_t(
                memory::allocator_t* allocator = context::current()->allocator) noexcept : _backing(allocator)  {
            assert(_backing);
        }

        template<class U> std_allocator_t(
                std_allocator_t<U> const& other) noexcept : _backing(other._backing) {
            assert(_backing);
        }

        value_type* allocate(std::size_t n) {
            return static_cast<value_type*>(_backing->allocate(
                n * sizeof(value_type),
                alignof(value_type)));
        }

        void deallocate(value_type* p, std::size_t) noexcept {
            _backing->deallocate(p);
        }

    private:
        memory::allocator_t* _backing;
    };

    template<class T, class U>
    bool operator==(std_allocator_t<T> const&, std_allocator_t<U> const&) noexcept {
        return true;
    }

    template<class T, class U>
    bool operator!=(std_allocator_t<T> const& x, std_allocator_t<U> const& y) noexcept {
        return !(x == y);
    }

}