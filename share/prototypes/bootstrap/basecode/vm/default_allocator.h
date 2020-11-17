// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include "vm_types.h"

namespace basecode::vm {

    struct heap_block_t {
        enum flags_t : uint8_t {
            none       = 0b00000000,
            allocated  = 0b00000001,
        };

        inline void mark_allocated() {
            flags |= flags_t::allocated;
        }

        inline void clear_allocated() {
            flags &= ~flags_t::allocated;
        }

        inline bool is_free() const {
            return (flags & flags_t::allocated) == 0;
        }

        inline bool is_allocated() const {
            return (flags & flags_t::allocated) != 0;
        }

        uint64_t size = 0;
        uint64_t address = 0;
        heap_block_t* prev = nullptr;
        heap_block_t* next = nullptr;
        uint8_t flags = flags_t::none;
    };

    ///////////////////////////////////////////////////////////////////////////

    class default_allocator : public allocator {
    public:
        default_allocator() = default;

        ~default_allocator() override;

        void reset() override;

        void initialize(
            uint64_t address,
            uint64_t size) override;

        uint64_t alloc(uint64_t size) override;

        uint64_t size(uint64_t address) override;

        uint64_t free(uint64_t address) override;

    private:
        void free_heap_block_list();

    private:
        uint64_t _size = 0;
        uint64_t _address = 0;
        heap_block_t* _head_heap_block = nullptr;
        std::unordered_map<uint64_t, heap_block_t*> _address_blocks {};
    };

};

