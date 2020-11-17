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

#include "proxy_allocator.h"

namespace basecode::memory {

    proxy_allocator_t::proxy_allocator_t(
            adt::string_t name,
            allocator_t* backing) : _name(std::move(name)),
                                    _backing(backing) {
    }

    proxy_allocator_t::~proxy_allocator_t() {
        assert(_backing->total_allocated() == 0);
    }

    void* proxy_allocator_t::allocate(
            uint32_t size,
            uint32_t align) {
        return _backing->allocate(size, align);
    }

    void proxy_allocator_t::deallocate(void* p) {
        _backing->deallocate(p);
    }

    std::string_view proxy_allocator_t::name() const {
        return _name.slice();
    }

    std::optional<uint32_t> proxy_allocator_t::total_allocated() {
        return _backing->total_allocated();
    }

    std::optional<uint32_t> proxy_allocator_t::allocated_size(void* p) {
        return _backing->allocated_size(p);
    }

}