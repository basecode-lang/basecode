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

#include "system.h"

namespace basecode::memory {

    template <uint32_t Buffer_Size>
    class temp_allocator_t : public allocator_t {
    public:
        explicit temp_allocator_t(
                allocator_t* backing = context::current()->allocator,
                uint32_t chunk_size = 4 * 1024) : _backing(backing),
                                                  _chunk_size(chunk_size) {
            _p = _start = _buffer;
            _end = _start + Buffer_Size;
            *(void**)_start = nullptr;
            _p += sizeof(void*);
        }

        ~temp_allocator_t() override {
            void* p = *(void**)_buffer;
            while (p) {
                void* next = *(void**)p;
                _backing->deallocate(p);
                p = next;
            }
        }

        void* allocate(
                uint32_t size,
                uint32_t align) override {
            _p = (uint8_t*)memory::align_forward(_p, align);
            if ((int)size > _end - _p) {
                uint32_t to_allocate = sizeof(void*) + size + align;
                if (to_allocate < _chunk_size)
                    to_allocate = _chunk_size;
                _chunk_size *= 2;
                auto p = _backing->allocate(to_allocate);
                *(void **)_start = p;
                _p = _start = (char *)p;
                _end = _start + to_allocate;
                *(void **)_start = nullptr;
                _p += sizeof(void *);
                _p = (uint8_t*)memory::align_forward(_p, align);
            }
            void* result = _p;
            _p += size;
            return result;
        }

        void deallocate(void*) override {
        }

        std::optional<uint32_t> total_allocated() override {
            return {};
        }

        std::optional<uint32_t> allocated_size(void*) override {
            return {};
        }

    private:
        uint8_t* _p{};
        uint8_t* _end{};
        uint8_t* _start{};
        uint32_t _chunk_size;
        allocator_t* _backing;
        uint8_t _buffer[Buffer_Size]{};
    };

}
