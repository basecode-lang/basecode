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
#include <basecode/hashing/murmur.h>
#include <basecode/memory/allocator.h>
#include "array.h"
#include "hashable.h"

namespace basecode::adt {

    template <typename K, typename V, std::uint32_t Initial_Size = 16>
    class hash_table_t final {
        static constexpr uint64_t hash_mask = 0b0011111111111111111111111111111111111111111111111111111111111111;

        struct hash_pair_t final {
            K key;
            V value;
        };

        enum hash_bucket_state_t {
            s_empty,
            s_filled,
            s_removed
        };

        struct hash_bucket_t final {
            size_t hash:62;
            size_t state:2;
        };

    public:
        hash_table_t(
                std::initializer_list<std::pair<K, V>> elements,
                memory::allocator_t* allocator = context::current()->allocator) : _allocator(allocator),
                                                                                  _pairs(allocator),
                                                                                  _buckets(allocator) {
            assert(_allocator);
            init();
            insert(elements);
        }

        hash_table_t(const hash_table_t& other) : _size(other._size),
                                                  _allocator(other._allocator),
                                                  _pairs(other._pairs),
                                                  _buckets(other._buckets) {
            assert(_allocator);
        }

        hash_table_t(hash_table_t&& other) noexcept : _size(other._size),
                                                      _allocator(other._allocator),
                                                      _pairs(std::move(other._pairs)),
                                                      _buckets(std::move(other._buckets)) {
            assert(_allocator);
            other._size = 0;
        }

        explicit hash_table_t(
                memory::allocator_t* allocator = context::current()->allocator) : _allocator(allocator),
                                                                                  _pairs(allocator),
                                                                                  _buckets(allocator) {
            assert(_allocator);
            init();
        }

        void clear() {
            _size = 0;
            reset_bucket_state(_buckets);
        }

        bool remove(K key) {
            auto hash = hash_key(key) & hash_mask;
            auto bucket_start_index = hash % _buckets.size();

            hash_pair_t* target_pair{};
            hash_bucket_t* target_bucket{};

            auto found = find_bucket_and_pair_by_matching_key(
                bucket_start_index,
                hash,
                key,
                &target_bucket,
                &target_pair);
            if (found) {
                target_bucket->hash = 0;
                target_bucket->state = hash_bucket_state_t::s_removed;
                --_size;
                return true;
            }

            return false;
        }

        decltype(auto) find(K key) const {
            size_t hash = hash_key(key) & hash_mask;
            auto bucket_start_index = hash % _buckets.size();

            hash_pair_t* target_pair{};
            hash_bucket_t* target_bucket{};

            auto found = find_bucket_and_pair_by_matching_key(
                bucket_start_index,
                hash,
                key,
                &target_bucket,
                &target_pair);

            if constexpr (std::is_pointer<V>::value) {
                V r = !found ? nullptr : target_pair->value;
                return r;
            } else {
                V* r = !found ? nullptr : &target_pair->value;
                return r;
            }
        }

        decltype(auto) insert(K key, V value) {
            if (_size * 3 > _buckets.size() * 2)
                rehash(_buckets.size() * 2);

            auto hash = hash_key(key) & hash_mask;
            auto bucket_start_index = hash % _buckets.size();

            hash_pair_t* target_pair{};
            hash_bucket_t* target_bucket{};

            auto found = find_available_bucket_and_pair(
                _buckets,
                _pairs,
                bucket_start_index,
                &target_bucket,
                &target_pair);
            assert(found);

            target_bucket->hash = hash;
            target_bucket->state = hash_bucket_state_t::s_filled;
            target_pair->key = key;
            target_pair->value = value;

            ++_size;

            if constexpr (std::is_pointer<V>::value) {
                return target_pair->value;
            } else {
                return &target_pair->value;
            }
        }

        decltype(auto) values() const {
            if constexpr (std::is_pointer<V>::value) {
                array_t<V> list(_allocator);
                list.resize(_size);

                size_t i = 0, j = 0;
                for (const auto& b : _buckets) {
                    if (b.state == hash_bucket_state_t::s_filled)
                        list[j++] = _pairs[i].value;
                    ++i;
                }

                return list;
            } else {
                array_t<V*> list(_allocator);
                list.resize(_size);

                size_t i = 0, j = 0;
                for (const auto& b : _buckets) {
                    if (b.state == hash_bucket_state_t::s_filled)
                        list[j++] = const_cast<V*>(&_pairs[i].value);
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

        [[nodiscard]] size_t size() const {
            return _size;
        }

        array_t<const hash_pair_t*> pairs() const {
            array_t<const hash_pair_t*> list(_allocator);
            list.resize(_size);

            size_t i = 0, j = 0;
            for (const auto& b : _buckets) {
                if (b.state == hash_bucket_state_t::s_filled)
                    list[j++] = &(_pairs[i]);
                ++i;
            }
            return list;
        }

        hash_table_t& operator=(const hash_table_t& other) {
            if (this != &other) {
                _size = other._size;
                _allocator = other._allocator;
                _pairs = other._pairs;
                _buckets = other._buckets;
            }
            return *this;
        }

        hash_table_t& operator=(hash_table_t&& other) noexcept {
            if (this != &other) {
                assert(_allocator == other._allocator);
                _pairs.clear();
                _buckets.clear();
                _size = other._size;
                _pairs = std::move(other._pairs);
                _buckets = std::move(other._buckets);
                other._size = 0;
            }
            return *this;
        }

    private:
        void init() {
            _pairs.resize(Initial_Size);
            _buckets.resize(Initial_Size);
            reset_bucket_state(_buckets);
        }

        void rehash(uint32_t new_bucket_count) {
            new_bucket_count = std::max<uint32_t>(
                std::max<uint32_t>(new_bucket_count, _size),
                Initial_Size);

            array_t<hash_pair_t> new_pairs(_allocator);
            new_pairs.resize(new_bucket_count);

            array_t<hash_bucket_t> new_buckets(_allocator);
            new_buckets.resize(new_bucket_count);
            reset_bucket_state(new_buckets);

            auto i = 0;
            for (auto& bucket : _buckets) {
                if (bucket.state != hash_bucket_state_t::s_filled) {
                    ++i;
                    continue;
                }

                hash_pair_t* target_pair{};
                hash_bucket_t* target_bucket{};
                auto bucket_start_index = bucket.hash % new_bucket_count;

                auto found = find_available_bucket_and_pair(
                    new_buckets,
                    new_pairs,
                    bucket_start_index,
                    &target_bucket,
                    &target_pair);

                assert(found);

                target_bucket->hash = bucket.hash;
                target_bucket->state = hash_bucket_state_t::s_filled;

                auto& original_pair = _pairs[i];
                target_pair->key = std::move(original_pair.key);
                target_pair->value = std::move(original_pair.value);

                ++i;
            }

            _pairs = std::move(new_pairs);
            _buckets = std::move(new_buckets);
        }

        bool find_available_bucket_and_pair(
                array_t<hash_bucket_t>& buckets,
                array_t<hash_pair_t>& pairs,
                uint32_t bucket_start_index,
                hash_bucket_t** target_bucket,
                hash_pair_t** target_pair) const {
            for (size_t i = bucket_start_index; i < buckets.size(); i++) {
                auto& bucket = buckets[i];
                if (bucket.state != hash_bucket_state_t::s_filled) {
                    *target_bucket = &bucket;
                    *target_pair = &pairs[i];
                    return true;
                }
            }

            for (size_t i = 0; i < bucket_start_index; i++) {
                auto& bucket = buckets[i];
                if (bucket.state != hash_bucket_state_t::s_filled) {
                    *target_bucket = &bucket;
                    *target_pair = &pairs[i];
                    return true;
                }
            }

            return false;
        }

        bool find_bucket_and_pair_by_matching_key(
                uint32_t bucket_start_index,
                size_t hash,
                K key,
                hash_bucket_t** target_bucket,
                hash_pair_t** target_pair) const {
            for (size_t i = bucket_start_index; i < _buckets.size(); i++) {
                auto& bucket = _buckets[i];
                switch (bucket.state) {
                    case hash_bucket_state_t::s_empty:
                        return false;
                    case hash_bucket_state_t::s_filled: {
                        if (bucket.hash == hash) {
                            auto& pair = _pairs[i];
                            if (pair.key == key) {
                                *target_pair = const_cast<hash_pair_t*>(&pair);
                                *target_bucket = const_cast<hash_bucket_t*>(&bucket);
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
                            auto& pair = _pairs[i];
                            if (pair.key == key) {
                                *target_pair = const_cast<hash_pair_t*>(&pair);
                                *target_bucket = const_cast<hash_bucket_t*>(&bucket);
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

        void reset_bucket_state(array_t<hash_bucket_t>& buckets) {
            for (size_t i = 0; i < buckets.size(); i++)
                buckets[i].state = 0;
        }

        void insert(std::initializer_list<std::pair<K, V>> elements) {
            reserve(elements.size());
            for (auto e : elements)
                insert(e.first, e.second);
        }

    private:
        size_t _size{};
        memory::allocator_t* _allocator;
        array_t<hash_pair_t, Initial_Size> _pairs;
        array_t<hash_bucket_t, Initial_Size> _buckets;
    };

}