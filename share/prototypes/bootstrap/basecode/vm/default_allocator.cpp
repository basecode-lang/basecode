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

#include "default_allocator.h"

namespace basecode::vm {

    default_allocator::~default_allocator() {
        free_heap_block_list();
    }

    void default_allocator::reset() {
        free_heap_block_list();

        _head_heap_block = new heap_block_t;
        _head_heap_block->size = _size;
        _head_heap_block->address = _address;
        _address_blocks.insert(std::make_pair(
            _head_heap_block->address,
            _head_heap_block));
    }

    void default_allocator::initialize(
            uint64_t address,
            uint64_t size) {
        _size = size;
        _address = address;
        reset();
    }

    void default_allocator::free_heap_block_list() {
        _address_blocks.clear();

        if (_head_heap_block == nullptr)
            return;

        auto current_block = _head_heap_block;
        while (current_block != nullptr) {
            auto next_block = current_block->next;
            delete current_block;
            current_block = next_block;
        }

        _head_heap_block = nullptr;
    }

    uint64_t default_allocator::alloc(uint64_t size) {
        uint64_t size_delta = size;
        heap_block_t* best_sized_block = nullptr;
        auto current_block = _head_heap_block;

        while (current_block != nullptr) {
            if (current_block->is_free()) {
                if (current_block->size == size) {
                    current_block->mark_allocated();
                    return current_block->address;
                } else if (current_block->size > size) {
                    auto local_size_delta = current_block->size - size;
                    if (best_sized_block == nullptr
                    ||  local_size_delta < size_delta) {
                        size_delta = local_size_delta;
                        best_sized_block = current_block;
                    }
                }
            }
            current_block = current_block->next;
        }

        if (best_sized_block != nullptr) {
            // if the block is over-sized by 64 bytes or less, just use it as-is
            if (size_delta <= 64) {
                best_sized_block->mark_allocated();
                return best_sized_block->address;
            } else {
                // otherwise, we need to split the block in two
                auto new_block = new heap_block_t;
                new_block->size = size;
                new_block->mark_allocated();
                new_block->prev = best_sized_block->prev;
                if (new_block->prev != nullptr)
                    new_block->prev->next = new_block;
                new_block->next = best_sized_block;
                new_block->address = best_sized_block->address;

                best_sized_block->prev = new_block;
                best_sized_block->address += size;
                best_sized_block->size -= size;

                if (new_block->prev == nullptr)
                    _head_heap_block = new_block;

                _address_blocks[new_block->address] = new_block;
                _address_blocks[best_sized_block->address] = best_sized_block;

                return best_sized_block->prev->address;
            }
        }

        return 0;
    }

    uint64_t default_allocator::free(uint64_t address) {
        auto it = _address_blocks.find(address);
        if (it == _address_blocks.end())
            return 0;

        heap_block_t* freed_block = it->second;
        auto freed_size = freed_block->size;
        freed_block->clear_allocated();

        // coalesce free blocks
        // first, we walk down the prev chain until we find a non-free block
        // then, we walk down the next chain until we find a non-free block
        // because blocks are known to be adjacent to each other in the heap,
        //          we then coalesce these blocks into one

        std::vector<heap_block_t*> delete_list {};
        uint64_t new_size = 0;

        auto first_free_block = freed_block;
        while (true) {
            auto prev = first_free_block->prev;
            if (prev == nullptr || !prev->is_free())
                break;
            first_free_block = prev;
        }

        auto last_free_block = freed_block;
        while (true) {
            auto next = last_free_block->next;
            if (next == nullptr || !next->is_free())
                break;
            last_free_block = next;
        }

        auto current_node = first_free_block;
        while (true) {
            delete_list.emplace_back(current_node);
            new_size += current_node->size;

            if (current_node == last_free_block)
                break;

            current_node = current_node->next;
        }

        if (first_free_block != last_free_block) {
            auto new_block = new heap_block_t;
            new_block->size = new_size;

            new_block->next = last_free_block->next;
            if (new_block->next != nullptr)
                new_block->next->prev = new_block;

            new_block->prev = first_free_block->prev;
            if (new_block->prev != nullptr)
                new_block->prev->next = new_block;

            new_block->address = first_free_block->address;

            for (auto block : delete_list) {
                _address_blocks.erase(block->address);
                delete block;
            }

            if (new_block->prev == nullptr)
                _head_heap_block = new_block;

            _address_blocks[new_block->address] = new_block;
        }

        return freed_size;
    }

    uint64_t default_allocator::size(uint64_t address) {
        auto it = _address_blocks.find(address);
        if (it == _address_blocks.end())
            return 0;
        return it->second->size;
    }

};