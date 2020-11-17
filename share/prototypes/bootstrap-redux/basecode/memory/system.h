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

    void shutdown();

    template <typename T>
    void destroy(T* obj) {
        return destroy<T>(
            context::current()->allocator,
            obj);
    }

    size_t os_page_size();

    allocator_t* default_allocator();

    template <typename T, typename... Args>
    T* construct_with_allocator(
            allocator_t* allocator,
            Args&&... args) {
        auto mem = allocator->allocate(sizeof(T), alignof(T));
        return new (mem) T(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    T* construct(Args&&... args) {
        return construct_with_allocator<T>(
            context::current()->allocator,
            std::forward<Args>(args)...);
    }

    allocator_t* default_scratch_allocator();

    template <typename T>
    void destroy(allocator_t* allocator, T* obj) {
        obj->~T();
        allocator->deallocate(obj);
    }

    void initialize(uint32_t scratch_buffer_size = 4*1024*1024);

}
