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

#include "hook.h"

namespace basecode::signals {

    static action_map_t* s_actions{};
    static memory::allocator_t* s_allocator{};

    void shutdown() {
        s_actions->~action_map_t();
        s_allocator->deallocate(s_actions);
    }

    bool initialize(
            result_t& r,
            memory::allocator_t* allocator) {
        s_allocator = allocator;

        auto mem = s_allocator->allocate(
            sizeof(action_map_t),
            alignof(action_map_t));
        s_actions = new (mem) action_map_t(s_allocator);

        return true;
    }

    static void on_signal(int sig) {
        auto action = s_actions->find(sig);
        if (!action)
            return;
        for (auto handler : action->handlers) {
            if (!(*handler)())
                return;
        }
    }

    bool hook(int sig, handler_t* handler) {
        auto action = s_actions->find(sig);
        if (!action) {
            action = s_actions->insert(sig, action_t(s_allocator));
            action->sigact.sa_flags = 0;
            sigemptyset(&action->sigact.sa_mask);
            action->sigact.sa_handler = on_signal;
            sigaction(sig, &action->sigact, nullptr);
        }
        action->handlers.add(handler);
        return true;
    }

}