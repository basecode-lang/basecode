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

#include <cstdint>
#include <cassert>
#include "context.h"

namespace basecode::context {

    static constexpr uint32_t stack_size = 512;

    thread_local context_t t_default{};
    thread_local uint32_t t_index = stack_size - 1;
    thread_local context_t* t_stack[stack_size];

    void pop() {
        assert(t_index < stack_size);
        t_index++;
    }

    void shutdown() {
        pop();
    }

    void initialize(
            memory::allocator_t* allocator,
            logging::logger_t* logger) {
        t_default.allocator = allocator;
        t_default.logger = logger;
        push(&t_default);
    }

    context_t* current() {
        return t_stack[t_index];
    }

    void push(context_t* ctx) {
        assert(t_index > 0);
        t_stack[--t_index] = ctx;
    }

}
