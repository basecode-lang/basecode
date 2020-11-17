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

#include "stack_allocator.h"

namespace basecode::memory {

    stack_allocator_t::~stack_allocator_t() {
        assert(_total_allocated == 0);
    }

    void* stack_allocator_t::allocate(
            uint32_t size,
            uint32_t align) {
        const uint32_t ts = size_with_padding(size, align);
        auto h = (header_t*) alloca(ts);
        void *p = data_pointer(h, align);
        fill(h, p, ts);
        _total_allocated += ts;
        return p;
    }

    void stack_allocator_t::deallocate(void* p) {
        if (!p)
            return;

        auto h = header(p);
        _total_allocated -= h->size;
    }

    std::optional<uint32_t> stack_allocator_t::total_allocated() {
        return _total_allocated;
    }

    std::optional<uint32_t> stack_allocator_t::allocated_size(void* p) {
        return header(p)->size;
    }

}