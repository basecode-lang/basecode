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

#include <utility>
#include "system.h"
#include "slab_allocator.h"

namespace basecode::memory {

    slab_allocator_t::slab_allocator_t(
            adt::string_t name,
            uint32_t size,
            uint32_t align,
            allocator_t* backing) : _name(std::move(name)),
                                    _backing(backing),
                                    _buffer_size(std::max<uint32_t>(size, 8)),
                                    _buffer_align(std::max<uint32_t>(align, 4)) {
        const auto slab_size = sizeof(slab_t) + alignof(slab_t);
        _maximum_buffers = (os_page_size() - slab_size) / _buffer_size;
    }

    slab_allocator_t::~slab_allocator_t() {
        const auto page_size = os_page_size();

        auto current = _front;
        while (current) {
            _backing->deallocate(current->page);
            _total_allocated -= page_size;
            current = current->next;
        }

        assert(_total_allocated == 0);
    }

    void slab_allocator_t::grow() {
        const auto page_size = os_page_size();

        auto mem = (char*)_backing->allocate(page_size);
        _total_allocated += page_size;
        _page_count++;

        auto slab = (slab_t*)align_forward(
            mem + page_size - (sizeof(slab_t) + alignof(slab_t)),
            alignof(slab_t));
        slab->next = slab->prev = nullptr;
        slab->buffer_count = 0;
        slab->page = mem;
        slab->free_list = mem;

        auto effective_size = _buffer_align * ((_buffer_size - 1) / _buffer_align + 1);
        char* last_buffer = mem + (effective_size * (_maximum_buffers - 1));

        auto i = 0;
        for (auto p = mem; p < last_buffer; p += effective_size) {
            *((void**) p) = p + effective_size;
            ++i;
        }

        move_front(slab);
    }

    void* slab_allocator_t::allocate(
            uint32_t size,
            uint32_t align) {
        if (!_front
        ||  _front->buffer_count == _maximum_buffers) {
            grow();
        }

        auto buffer = _front->free_list;
        _front->free_list = *((void**)buffer);
        _front->buffer_count++;

        if (_front->buffer_count == _maximum_buffers)
            move_back(_front);

        return buffer;
    }

    void slab_allocator_t::remove(slab_t* slab) {
        if (slab->next)
            slab->next->prev = slab->prev;

        if (slab->prev)
            slab->prev->next = slab->next;

        if (_front == slab) {
            _front = slab->next;
        }

        if (_back == slab) {
            _back = slab->prev;
        }
    }

    void slab_allocator_t::deallocate(void* p) {
        const auto page_size = os_page_size();
        const auto slab_size = page_size - (sizeof(slab_t) + alignof(slab_t));

        auto current = _front;
        while (current) {
            if (p >= current->page
            &&  p <= ((char*)current->page) + slab_size) {
                break;
            }
            current = current->next;
        }

        assert(current);

        auto slab = (slab_t*)align_forward(
            (char*)current->page + page_size - (sizeof(slab_t) + alignof(slab_t)),
            alignof(slab_t));
        *((void**)p) = slab->free_list;
        slab->free_list = p;
        slab->buffer_count--;

        if (slab->buffer_count == 0) {
            remove(slab);
            _backing->deallocate(slab->page);
            _total_allocated -= page_size;
        }

        if (slab->buffer_count == _maximum_buffers - 1)
            move_front(slab);
    }

    void slab_allocator_t::move_back(slab_t* slab) {
        if (_back == slab) return;

        remove(slab);

        slab->next = nullptr;
        slab->prev = _back;
        _back->next = slab;

        _back = slab;
    }

    void slab_allocator_t::move_front(slab_t* slab) {
        if (_front == slab) return;

        remove(slab);

        slab->prev = nullptr;

        if (_front) {
            _front->prev = slab;
            slab->next = _front;
            if (!_back)
                _back = _front;
        } else {
            _back = slab;
        }

        _front = slab;
    }

    std::string_view slab_allocator_t::name() const {
        return _name.slice();
    }

    std::optional<uint32_t> slab_allocator_t::total_allocated() {
        return _total_allocated;
    }

    std::optional<uint32_t> slab_allocator_t::allocated_size(void*) {
        return _buffer_size;
    }

}