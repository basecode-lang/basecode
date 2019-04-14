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

#include <memory>
#include <cstdint>

namespace basecode::common {

    template <typename T>
    class memory_pool {
    public:
        explicit memory_pool(size_t arena_size) : _arena_size(arena_size) {
            _arena = std::unique_ptr<arena_t>(new arena_t(arena_size));
            _free_list = _arena->storage();
        }

        memory_pool(const memory_pool&) = delete;

        template <typename... Args>
        T* alloc(Args&&... args) {
            if (_free_list == nullptr) {
                std::unique_ptr<arena_t> new_arena(new arena_t(_arena_size));
                new_arena->next_arena(std::move(_arena));
                _arena.reset(new_arena.release());
                _free_list = _arena->storage();
            }

            auto current_item = _free_list;
            _free_list = current_item->next_item();

            auto result = current_item->storage();
            new (result) T(std::forward<Args>(args)...);
            return result;
        }

        void free(T* value) {
            value->T::~T();

            auto current_item = item_t::storage_to_item(value);
            current_item->next_item(_free_list);
            _free_list = current_item;
        }

    private:
        union item_t {
        public:
            static item_t* storage_to_item(T* value) {
                return reinterpret_cast<item_t*>(value);
            }

            item_t* next_item() const {
                return _next;
            }

            void next_item(item_t* value) {
                _next = value;
            }

            T* storage() {
                return reinterpret_cast<T*>(_data);
            }

        private:
            using storage_type = uint8_t[sizeof(T)];

            alignas(alignof(T)) storage_type _data;
            item_t* _next;
        };

        struct arena_t {
            explicit arena_t(size_t size) : _storage(new item_t[size]) {
                for (size_t i = 1; i < size; i++) {
                    _storage[i - 1].next_item(&_storage[i]);
                }
                _storage[size - 1].next_item(nullptr);
            }

            item_t* storage() const {
                return _storage.get();
            }

            void next_arena(std::unique_ptr<arena_t>&& n) {
                _next.reset(n.release());
            }

        private:
            std::unique_ptr<arena_t> _next {};
            std::unique_ptr<item_t[]> _storage;
        };

    private:
        size_t _arena_size;
        item_t* _free_list;
        std::unique_ptr<arena_t> _arena;
    };

}

