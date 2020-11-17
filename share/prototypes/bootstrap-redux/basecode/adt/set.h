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
#include <algorithm>
#include <initializer_list>
#include <basecode/memory/system.h>
#include <basecode/hashing/murmur.h>
#include <basecode/memory/allocator.h>
#include "array.h"
#include "hashable.h"

namespace basecode::adt {

    template <typename T, std::uint32_t Initial_Size = 16>
    class set_t final {
        static constexpr uint64_t hash_mask = 0b0011111111111111111111111111111111111111111111111111111111111111;

        struct hash_value_t final {
            T value{};
            uint32_t count{};
        };

        enum hash_bucket_state_t {
            s_empty,
            s_filled,
            s_removed
        };

        struct hash_bucket_t final {
            hash_bucket_t() : hash(0), state(0) {}
            size_t hash:62;
            size_t state:2;
        };

    public:
        set_t(
                std::initializer_list<T> elements,
                memory::allocator_t* allocator = context::current()->allocator) : _allocator(allocator) {
            assert(_allocator);
            init();
            insert(elements);
        }

        set_t(const set_t& other) : _size(other._size),
                                    _allocator(other._allocator),
                                    _values(std::move(other._values)),
                                    _buckets(std::move(other._buckets)) {
            assert(_allocator);
        }

        explicit set_t(
                memory::allocator_t* allocator = context::current()->allocator) : _allocator(allocator) {
            assert(_allocator);
            init();
        }

        bool has(const T& value) {
            size_t hash = hash_key(value) & hash_mask;
            auto bucket_start_index = hash % _buckets.size();

            hash_value_t* target_pair{};
            hash_bucket_t* target_bucket{};

            return find_bucket_and_pair_by_matching_value(
                bucket_start_index,
                hash,
                value,
                &target_bucket,
                &target_pair);
        }

        bool insert(const T& value) {
            if (_size * 3 > _buckets.size() * 2)
                rehash(_buckets.size() * 2);

            size_t hash = hash_key(value) & hash_mask;
            auto bucket_start_index = hash % _buckets.size();

            hash_value_t* target_value{};
            hash_bucket_t* target_bucket{};

            auto found = find_bucket_and_pair_by_matching_value(
                bucket_start_index,
                hash,
                value,
                &target_bucket,
                &target_value);
            if (found) {
                target_value->count++;
                return false;
            }

            found = find_available_bucket_and_value(
                _buckets,
                _values,
                bucket_start_index,
                &target_bucket,
                &target_value);
            assert(found);

            target_bucket->hash = hash;
            target_bucket->state = hash_bucket_state_t::s_filled;
            target_value->value = value;
            target_value->count = 1;

            ++_size;

            return true;
        }

        bool remove(const T& value) {
            auto hash = hash_key(value) & hash_mask;
            auto bucket_start_index = hash % _buckets.size();

            hash_value_t* target_value{};
            hash_bucket_t* target_bucket{};

            auto found = find_bucket_and_pair_by_matching_value(
                bucket_start_index,
                hash,
                value,
                &target_bucket,
                &target_value);
            if (found) {
                target_bucket->hash = 0;
                target_bucket->state = hash_bucket_state_t::s_removed;
                --_size;
                return true;
            }

            return false;
        }

        uint32_t count(const T& value) {
            size_t hash = hash_key(value) & hash_mask;
            auto bucket_start_index = hash % _buckets.size();

            hash_value_t* target_value{};
            hash_bucket_t* target_bucket{};

            auto found = find_bucket_and_pair_by_matching_value(
                bucket_start_index,
                hash,
                value,
                &target_bucket,
                &target_value);
            if (!found) return 0;
            return target_value->count;
        }

        decltype(auto) elements() const {
            if constexpr (std::is_pointer<T>::value) {
                array_t<T> list(_allocator);
                list.resize(_size);

                size_t i = 0, j = 0;
                for (const auto& b : _buckets) {
                    if (b.state == hash_bucket_state_t::s_filled)
                        list[j++] = _values[i].value;
                    ++i;
                }

                return list;
            } else {
                array_t<T*> list(_allocator);
                list.resize(_size);

                size_t i = 0, j = 0;
                for (const auto& b : _buckets) {
                    if (b.state == hash_bucket_state_t::s_filled)
                        list[j++] = const_cast<T*>(&_values[i].value);
                    ++i;
                }

                return list;
            }
        }

        void reserve(uint32_t new_size) {
            rehash(new_size * 3 / 2);
        }

        [[nodiscard]] bool empty() const {
            return _size == 0;
        }

        [[nodiscard]] uint32_t size() const {
            return _size;
        }

    private:
        void init() {
            _values.resize(Initial_Size);
            _buckets.resize(Initial_Size);
            reset_bucket_state(_buckets);
        }

        void rehash(uint32_t new_bucket_count) {
            new_bucket_count = std::max(
                std::max(new_bucket_count, _size),
                Initial_Size);

            array_t<hash_value_t> new_values(_allocator);
            new_values.resize(new_bucket_count);

            array_t<hash_bucket_t> new_buckets(_allocator);
            new_buckets.resize(new_bucket_count);
            reset_bucket_state(new_buckets);

            auto i = 0;
            for (auto& bucket : _buckets) {
                if (bucket.state != hash_bucket_state_t::s_filled) {
                    ++i;
                    continue;
                }

                hash_value_t* target_value{};
                hash_bucket_t* target_bucket{};
                auto bucket_start_index = bucket.hash % new_bucket_count;

                auto found = find_available_bucket_and_value(
                    new_buckets,
                    new_values,
                    bucket_start_index,
                    &target_bucket,
                    &target_value);

                assert(found);

                target_bucket->hash = bucket.hash;
                target_bucket->state = hash_bucket_state_t::s_filled;

                auto& original_value = _values[i];
                target_value->value = std::move(original_value.value);
                target_value->count = std::move(original_value.count);

                ++i;
            }

            _values = std::move(new_values);
            _buckets = std::move(new_buckets);
        }

        bool find_available_bucket_and_value(
                array_t<hash_bucket_t>& buckets,
                array_t<hash_value_t>& values,
                uint32_t bucket_start_index,
                hash_bucket_t** target_bucket,
                hash_value_t** target_value) {
            for (size_t i = bucket_start_index; i < buckets.size(); i++) {
                auto& bucket = buckets[i];
                if (bucket.state != hash_bucket_state_t::s_filled) {
                    *target_bucket = &bucket;
                    *target_value = &values[i];
                    return true;
                }
            }

            for (size_t i = 0; i < bucket_start_index; i++) {
                auto& bucket = buckets[i];
                if (bucket.state != hash_bucket_state_t::s_filled) {
                    *target_bucket = &bucket;
                    *target_value = &values[i];
                    return true;
                }
            }

            return false;
        }

        bool find_bucket_and_pair_by_matching_value(
                uint32_t bucket_start_index,
                size_t hash,
                T value,
                hash_bucket_t** target_bucket,
                hash_value_t** target_value) {
            for (size_t i = bucket_start_index; i < _buckets.size(); i++) {
                auto& bucket = _buckets[i];
                switch (bucket.state) {
                    case hash_bucket_state_t::s_empty:
                        return false;
                    case hash_bucket_state_t::s_filled: {
                        if (bucket.hash == hash) {
                            auto& hash_value = _values[i];
                            if (hash_value.value == value) {
                                *target_bucket = &bucket;
                                *target_value = &hash_value;
                                return true;
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            for (size_t i = 0; i < bucket_start_index; i++) {
                auto& bucket = _buckets[i];
                switch (bucket.state) {
                    case hash_bucket_state_t::s_empty:
                        return false;
                    case hash_bucket_state_t::s_filled: {
                        if (bucket.hash == hash) {
                            auto& hash_value = _values[i];
                            if (hash_value.value == value) {
                                *target_bucket = &bucket;
                                *target_value = &hash_value;
                                return true;
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            return false;
        }

        void insert(std::initializer_list<T> elements) {
            reserve(elements.size());
            for (const auto& e : elements)
                insert(const_cast<T&&>(e));
        }

        void reset_bucket_state(array_t<hash_bucket_t>& buckets) {
            for (size_t i = 0; i < buckets.size(); i++)
                buckets[i].state = 0;
        }

    private:
        uint32_t _size{};
        memory::allocator_t* _allocator;
        array_t<hash_value_t, Initial_Size> _values{};
        array_t<hash_bucket_t, Initial_Size> _buckets{};
    };

}