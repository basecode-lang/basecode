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

#include <utility>
#include <basecode/types.h>
#include <basecode/utf8/rune.h>
#include <basecode/adt/hash_table.h>
#include <basecode/memory/object_pool.h>

namespace basecode::adt {

    template <typename V>
    class trie_map_t final {
    public:
        struct tree_node_t;

        struct pair_t final {
            V value;
            adt::string_t key{};

            bool operator<(const pair_t& other) const {
                return key < other.key;
            }
        };

        struct node_t final {
            node_t(V data, tree_node_t* tree) : data(data), tree(tree) {}
            V data;
            bool leaf{};
            tree_node_t* tree;
        };

        struct tree_node_t final {
            explicit tree_node_t(memory::allocator_t* allocator) : children(allocator) {}
            adt::hash_table_t<utf8::rune_t, node_t*> children;
        };

        explicit trie_map_t(memory::allocator_t* allocator = context::current()->allocator) : _tree_root(allocator),
                                                                                              _storage(allocator),
                                                                                              _allocator(allocator) {
            assert(_allocator);
        }

        trie_map_t(
                std::initializer_list<std::pair<std::string_view, V>> elements,
                memory::allocator_t* allocator = context::current()->allocator) : _tree_root(allocator),
                                                                                  _storage(allocator),
                                                                                  _allocator(allocator) {
            assert(_allocator);
            insert(elements);
        }

        void walk(
                const node_t* node,
                string_t& str,
                adt::array_t<pair_t>& pairs) const {
            if (node->leaf) {
                pairs.add(pair_t{
                    .key = str,
                    .value = node->data
                });
            }
            const auto& children = node->tree->children.pairs();
            for (auto child : children) {
                str.append((char)child->key);
                walk(child->value, str, pairs);
                str.erase(str.end() - 1);
            }
        }

        adt::array_t<pair_t> pairs() const {
            adt::array_t<pair_t> pairs{};
            adt::string_t str{};

            walk(&_root, str, pairs);
            std::sort(pairs.begin(), pairs.end());

            return pairs;
        }

        void insert(std::string_view key, V value) {
            assert(!key.empty());

            auto current_node = &_root;

            size_t i = 0;
            const auto key_length = key.length() - 1;
            for (const char c : key) {
                utf8::rune_t rune(c);
                auto& children = current_node->tree->children;
                auto node = children.find(rune);
                if (!node) {
                    current_node = _storage.construct<node_t>(V{}, nullptr);
                    if (i <= key_length)
                        current_node->tree = _storage.construct<tree_node_t>(_allocator);
                    children.insert(rune, current_node);
                } else {
                    current_node = node;
                }
                ++i;
            }

            current_node->leaf = true;
            current_node->data = value;
        }

        decltype(auto) search(std::string_view key) const {
            node_t* current_node = nullptr;

            for (const char c : key) {
                current_node = find(current_node, utf8::rune_t(c));
                if (!current_node)
                    break;
            }

            if constexpr (std::is_pointer<V>::value) {
                V r = !current_node ? nullptr : current_node->data;
                return r;
            } else {
                V* r = !current_node ? nullptr : &current_node->data;
                return r;
            }
        }

        node_t* find(node_t* node, const utf8::rune_t& rune) const {
            auto current_node = node ? node : &_root;
            auto& children = current_node->tree->children;
            auto child_node = children.find(rune);
            if (!child_node)
                return nullptr;
            return child_node;
        }

        trie_map_t& operator =(std::initializer_list<std::pair<std::string_view, V>> elements) {
            insert(elements);
            return *this;
        }

    private:
        void insert(std::initializer_list<std::pair<std::string_view, V>> elements) {
            for (const auto& e : elements)
                insert(e.first, e.second);
        }

    private:
        tree_node_t _tree_root;
        node_t _root{V{}, &_tree_root};
        memory::object_pool_t _storage;
        memory::allocator_t* _allocator;
    };

}