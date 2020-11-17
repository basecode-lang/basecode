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

#include <csignal>
#include <basecode/result.h>
#include <basecode/adt/hash_table.h>

namespace basecode::signals {

    struct handler_t {
        bool operator ()() {
            return on_invoke();
        }

    protected:
        virtual bool on_invoke() = 0;
    };

    struct action_t final {
        explicit action_t(memory::allocator_t* allocator = context::current()->allocator) : handlers(allocator) {
        }
        struct sigaction sigact{};
        adt::array_t<handler_t*> handlers;
    };

    using action_map_t = adt::hash_table_t<int, action_t>;

    void shutdown();

    bool initialize(
        result_t& r,
        memory::allocator_t* allocator = memory::default_allocator());

    bool hook(int sig, handler_t* handler);

}