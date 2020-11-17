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

#include "scratch_allocator.h"

namespace basecode::memory {

    scratch_allocator_t::scratch_allocator_t(
            uint32_t size,
            allocator_t* backing) : _backing(backing) {
        _begin = (uint8_t*)_backing->allocate(size);
        _end = _begin + size;
        _allocate = _begin;
        _free = _begin;
    }

    scratch_allocator_t::~scratch_allocator_t() {
        assert(_free == _allocate);
        _backing->deallocate(_begin);
    }

    bool scratch_allocator_t::in_use(void* p) {
        if (_free == _allocate)
            return false;
        if (_allocate > _free)
            return p >= _free && p < _allocate;
        return p >= _free || p < _allocate;
    }

    void* scratch_allocator_t::allocate(
            uint32_t size,
            uint32_t align) {
        if (align < 4) align = 4;
        size = ((size + 3)/4)*4;

        auto p = _allocate;
        auto h = (header_t *)p;
        auto data = (uint8_t*)data_pointer(h, align);
        p = data + size;

        // reached the end of the buffer, wrap around to the beginning
        if (p > _end) {
            h->size = (_end - (uint8_t*)h) | 0x80000000u;

            p = _begin;
            h = (header_t *)p;
            data = (uint8_t*)data_pointer(h, align);
            p = data + size;
        }

        // if the buffer is exhausted use the backing allocator instead
        if (in_use(p)) {
            return _backing->allocate(size, align);
        }

        fill(h, data, p - (uint8_t*)h);
        _allocate = p;

        auto header_size = h->size & 0x7fffffffu;
        _total_allocated += header_size;
        return data;
    }

    void scratch_allocator_t::deallocate(void* p) {
        if (!p)
            return;

        if (p < _begin || p >= _end) {
            _backing->deallocate(p);
            return;
        }

        // mark this slot as free
        auto h = header(p);
        assert((h->size & 0x80000000u) == 0);
        h->size = h->size | 0x80000000u;

        auto header_size = h->size & 0x7fffffffu;
        _total_allocated -= header_size;

        // advance the free pointer past all free slots
        while (_free != _allocate) {
            h = (header_t *)_free;
            if ((h->size & 0x80000000u) == 0)
                break;

            _free += h->size & 0x7fffffffu;
            if (_free == _end)
                _free = _begin;
        }
    }

    std::optional<uint32_t> scratch_allocator_t::total_allocated() {
        return _total_allocated;
    }

    std::optional<uint32_t> scratch_allocator_t::allocated_size(void* p) {
        auto h = header(p);
        return h->size - ((char *)p - (char *)h);
    }

}