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

    template <std::uint32_t Frame_Size = 4096>
    class frame_allocator_t : public allocator_t {
    public:
        explicit frame_allocator_t(
            allocator_t* backing = context::current()->allocator) : _backing(backing) {
        }

        ~frame_allocator_t() override {
            if (_block != nullptr) {
                auto current_block = _block;
                while (current_block != nullptr) {
                    _backing->deallocate(current_block);
                    _total_allocated -= _block_size;
                    auto prev_ptr = static_cast<uint64_t*>(_block);
                    current_block = reinterpret_cast<void*>(*prev_ptr);
                }
            }
            assert(_total_allocated == 0);
        }

        void* allocate(
                uint32_t size,
                uint32_t align) override {
            if (_block == nullptr
            ||  _offset + size > _block_size) {
                auto old_block = _block;
                _block = _backing->allocate(_block_size, align);
                _total_allocated += _block_size;
                _offset = 8;

                if (old_block != nullptr) {
                    auto prev_ptr = static_cast<uint64_t*>(_block);
                    *prev_ptr = reinterpret_cast<uint64_t>(old_block);
                }
            }

            auto new_data = static_cast<char*>(_block) + _offset;
            _offset += size;
            _offset = numbers::align(_offset, align);

            return new_data;
        }

        void deallocate(void* p) override {
        }

        std::optional <uint32_t> total_allocated() override {
            return _total_allocated;
        }

        std::optional <uint32_t> allocated_size(void* p) override {
            return _block_size;
        }

    private:
        void* _block{};
        uint32_t _offset{};
        allocator_t* _backing;
        uint32_t _total_allocated{};
        const uint32_t _block_size = Frame_Size;
    };

}