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

#include <cstdint>
#include <optional>
#include <algorithm>
#include <initializer_list>
#include <basecode/numbers/bytes.h>
#include <basecode/memory/system.h>

namespace basecode::adt {

    template <typename T, std::uint32_t Initial_Capacity = 16>
    class array_t final {
    public:
        explicit array_t(
                memory::allocator_t* allocator = context::current()->allocator) : _allocator(allocator) {
            assert(_allocator);
        }

        array_t(
                std::initializer_list<T> elements,
                memory::allocator_t* allocator = context::current()->allocator) : _allocator(allocator) {
            assert(_allocator);
            insert(elements);
        }

        array_t(
                std::initializer_list<const char*> elements,
                memory::allocator_t* allocator = context::current()->allocator) : _allocator(allocator) {
            assert(_allocator);
            insert(elements);
        }

        array_t(const array_t& other) : _allocator(other._allocator) {
            assert(_allocator);
            const auto n = other._size;
            grow(n);
            for (size_t i = 0; i < n; i++)
                _data[_size++] = other._data[i];
        }

        array_t(array_t&& other) noexcept : _data(other._data),
                                            _size(other._size),
                                            _capacity(other._capacity),
                                            _allocator(other._allocator) {
            other._data = nullptr;
            other._size = other._capacity = 0;
        }

        ~array_t() {
            if (_data)
                deallocate_data();
        }

        T* end() {
            return _data + _size;
        }

        T& back() {
            return _data[_size - 1];
        }

        T* rend() {
            return _data;
        }

        T* begin() {
            return _data;
        }

        T* rbegin() {
            return _data + _size;
        }

        void pop() {
            --_size;
        }

        void trim() {
            set_capacity(_size);
        }

        void clear() {
            if constexpr (!std::is_trivially_constructible<T>::value) {
                // XXX: this seems expensive but i don't see
                //      another way to cleanly handle it
                for (size_t i = 0; i < _size; i++) {
                    (&_data[i])->~T();
                    new (_data + i) T();
                }
            } else {
                std::memset(_data, 0, _capacity * sizeof(T));
            }
            resize(0);
        }

        void add(T&& value) {
            if (_size + 1 > _capacity) grow();
            _data[_size++] = value;
        }

        const T* end() const {
            return _data + _size;
        }

        const T* rend() const {
            return _data;
        }

        const T& back() const {
            return _data[_size - 1];
        }

        T* erase(const T* it) {
            const auto offset = it - _data;
            (&_data[offset])->~T();
            std::memmove(
                _data + offset,
                _data + offset + 1,
                (_size - offset - 1) * sizeof(T));
            _size--;
            return _data + offset;
        }

        const T* begin() const {
            return _data;
        }

        const T* rbegin() const {
            return _data + _size;
        }

        void add(const T& value) {
            if (_size + 1 > _capacity) grow();
            _data[_size++] = value;
        }

        template <typename... Args>
        T& emplace(Args&&... args) {
            if (_size + 1 > _capacity) grow();
            auto& value = *(new (_data + _size) T(std::forward<Args>(args)...));
            _size++;
            return value;
        }

        T& operator[](size_t index) {
            return _data[index];
        }

        void resize(uint32_t new_size) {
            if (new_size > _capacity) grow(new_size);
            _size = new_size;
        }

        [[nodiscard]] bool empty() const {
            return _size == 0;
        }

        T* insert(const T* it, const T& v) {
            const auto offset = it - _data;
            if (_size == _capacity)
                reserve(_size + 1);
            if (offset < _size) {
                std::memmove(
                    _data + offset + 1,
                    _data + offset,
                    (_size - offset) * sizeof(T));
            }
            _data[offset] = v;
            _size++;
            return _data + offset;
        }

        [[nodiscard]] uint32_t size() const {
            return _size;
        }

        void reserve(uint32_t new_capacity) {
            if (new_capacity > _capacity)
                set_capacity(new_capacity);
        }

        [[nodiscard]] uint32_t capacity() const {
            return _capacity;
        }

        const T& operator[](size_t index) const {
            return _data[index];
        }

        array_t& operator=(const array_t& other) {
            if (this != &other) {
                if (!_allocator)
                    _allocator = other._allocator;
                auto n = other._size;
                grow(n);
                for (size_t i = 0; i < n; i++) {
                    if (i < _size) (&_data[i])->~T();
                    _data[i] = other._data[i];
                }
                _size = n;
            }
            return *this;
        }

        T* erase(const T* it_begin, const T* it_end) {
            const auto count = it_end - it_begin;
            const auto offset = it_begin - _data;
            for (size_t i = 0; i < count; i++)
                (&_data[offset + i])->~T();
            std::memmove(
                _data + offset,
                _data + offset + count,
                (_size - offset - count) * sizeof(T));
            _size -= count;
            return _data + offset;
        }

        array_t& operator=(array_t&& other) noexcept {
            if (this != &other) {
                assert(_allocator == other._allocator);
                deallocate_data();
                _data = other._data;
                _size = other._size;
                _capacity = other._capacity;
                other._data = nullptr;
                other._size = other._capacity = 0;
            }
            return *this;
        }

        [[nodiscard]] bool contains(const T& v) const {
            const T* data = _data;
            const T* data_end = _data + _size;
            while (data < data_end) if (*data++ == v) return true;
            return false;
        }

    private:
        void deallocate_data() {
            if constexpr (!std::is_trivially_constructible<T>::value) {
                for (size_t i = 0; i < _size; i++)
                    (&_data[i])->~T();
            }
            _allocator->deallocate(_data);
        }

        void set_capacity(uint32_t new_capacity) {
            if (new_capacity == _capacity) return;

            if (new_capacity < _size)
                resize(new_capacity);

            T* new_data{};
            if (new_capacity > 0) {
                new_data = (T*)_allocator->allocate(
                    new_capacity * sizeof(T),
                    alignof(T));
                if (_data)
                    std::memcpy(new_data, _data, _size * sizeof(T));
                if constexpr (!std::is_trivially_constructible<T>::value) {
                    for (size_t i = _size; i < new_capacity; i++)
                        new (new_data + 1) T();
                }
            }
            _allocator->deallocate(_data);
            _data = new_data;
            _capacity = new_capacity;
        }

        void insert(std::initializer_list<T> elements) {
            reserve(elements.size());
            for (const auto& e : elements)
                _data[_size++] = e;
        }

        void grow(uint32_t min_capacity = Initial_Capacity) {
            set_capacity(std::max(_capacity * 2 + 8, min_capacity));
        }

        void insert(std::initializer_list<const char*> elements) {
            reserve(elements.size());
            for (const auto& e : elements)
                _data[_size++] = e;
        }

    private:
        T* _data{};
        uint32_t _size{};
        uint32_t _capacity{};
        memory::allocator_t* _allocator;
    };

}