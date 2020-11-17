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

#include "array.h"

namespace basecode::adt {

    template <typename T, std::uint32_t Initial_Capacity = 16>
    class queue_t final {
    public:
        queue_t(
                std::initializer_list<T> elements,
                memory::allocator_t* allocator = context::current()->allocator) : _allocator(allocator),
                                                                                  _data(allocator) {
            assert(_allocator);
            insert(elements);
        }

        queue_t(const queue_t& other) : _size(other._size),
                                        _offset(other._offset),
                                        _allocator(other._allocator),
                                        _data(std::move(other._data)) {
            assert(_allocator);
        }

        explicit queue_t(
                memory::allocator_t* allocator = context::current()->allocator) : _allocator(allocator),
                                                                                  _data(allocator) {
            assert(_allocator);
        }

        T* end() {
            uint32_t end = _offset + _size;
            return end > _data.size() ? _data.end() : _data.begin() + end;
        }

        T* begin() {
            return _data.begin() + _offset;
        }

        void dequeue_back() {
            if (_size > 0)
                --_size;
        }

        void dequeue_front() {
            if (_size > 0) {
                _offset = (_offset + 1) % _data.size();
                --_size;
            }
        }

        const T* end() const {
            uint32_t end = _offset + _size;
            return end > _data.size() ? _data.end() : _data.begin() + end;
        }

        const T* begin() const {
            return _data.begin() + _offset;
        }

        void reserve(uint32_t size) {
            if (size > _size)
                increase_capacity(size);
        }

        void consume(uint32_t count) {
            if (_size >= count) {
                _offset = (_offset + count) % _data.size();
                _size -= count;
            }
        }

        T& operator[](uint32_t index) {
            return _data[index];
        }

        void enqueue_back(const T& value) {
            if (!space()) grow();
            _data[_size++] = value;
        }

        void enqueue_front(const T& value) {
            if (!space()) grow();
            _offset = (_offset - 1 + _data.size()) % _data.size();
            ++_size;
            _data[0] = value;
        }

        [[nodiscard]] uint32_t size() const {
            return _size;
        }

        [[nodiscard]] uint32_t space() const {
            return _data.size() - _size;
        }

        const T& operator[](uint32_t index) const {
            return _data[index];
        }

    private:
        void increase_capacity(uint32_t new_capacity) {
            uint32_t end = _data.size();
            _data.resize(new_capacity);
            if (_offset + _size > end) {
                uint32_t end_items = end - _offset;
                std::memmove(
                    _data.begin() + new_capacity - end_items,
                    _data.end() + _offset,
                    end_items * sizeof(T));
                _offset += new_capacity - end;
            }
        }

        void insert(std::initializer_list<T> elements) {
            reserve(elements.size());
            for (const auto& e : elements)
                enqueue_back(e);
        }

        void grow(uint32_t min_capacity = Initial_Capacity) {
            increase_capacity(std::max(_data.size * 2 + 8, min_capacity));
        }

    private:
        uint32_t _size{};
        uint32_t _offset{};
        memory::allocator_t* _allocator;
        array_t<T, Initial_Capacity> _data;
    };

}