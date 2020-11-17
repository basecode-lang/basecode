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

namespace basecode::memory {
    class allocator_t;
}

namespace basecode::logging {
    class logger_t;
}

namespace basecode::context {

    struct context_t;

    void pop();

    void shutdown();

    void initialize(
        memory::allocator_t* allocator,
        logging::logger_t* logger = nullptr);

    context_t* current();

    void push(context_t* ctx);

    struct context_t final {
        ~context_t() { pop(); }
        void* user{};
        logging::logger_t* logger{};
        memory::allocator_t* allocator{};
    };

}
