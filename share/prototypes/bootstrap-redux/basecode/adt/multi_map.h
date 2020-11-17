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

#include <basecode/adt/array.h>
#include <basecode/context/context.h>
#include <basecode/memory/allocator.h>

namespace basecode::adt {

    template <typename K, typename V>
    class multi_map_t final {
        enum class node_color_t : uint8_t {
            red,
            black
        };

        struct list_item_t final {
            V value;
            list_item_t* next{};
        };

        struct node_t final {
            K key;
            node_t* left{};
            uint32_t size{};
            node_t* right{};
            node_t* parent{};
            node_color_t color{};
            list_item_t* list_items{};
        };

        using walk_callback_t = std::function<bool (node_t*)>;

    public:
        explicit multi_map_t(
                memory::allocator_t* allocator = context::current()->allocator) : _allocator(allocator) {
            assert(_allocator);
        }

        ~multi_map_t() {
            clear();
        }

        void clear() {
            if (_size == 0) return;
            auto i = 0;
            node_t* to_free[_size];
            walk(_root, [&](auto node) {
                to_free[i++] = node;
                return true;
            });
            to_free[i] = _root;

            for (auto node : to_free) {
                clear_list_item(node->list_items);
                if constexpr (!std::is_trivially_destructible<K>::value)
                    node->~node_t();
                _allocator->deallocate(node);
            }
            _size = 0;
            _root = nullptr;
        }

        decltype(auto) find(K key) {
            if constexpr (std::is_pointer<V>::value) {
                auto values = array_t<V>(_allocator);
                auto node = search(_root, key);
                if (node) {
                    values.reserve(node->size);
                    auto current_item = node->list_items;
                    while (current_item) {
                        values.add(current_item->value);
                        current_item = current_item->next;
                    }
                }
                return values;
            } else {
                auto values = array_t<V*>(_allocator);
                auto node = search(_root, key);
                if (node) {
                    values.reserve(node->size);
                    auto current_item = node->list_items;
                    while (current_item) {
                        values.add(&current_item->value);
                        current_item = current_item->next;
                    }
                }
                return values;
            }
        }

        void insert(K key, V value) {
            auto new_item = make_list_item(value);
            auto node = search(_root, key);
            if (node == nullptr) {
                auto new_node = make_node(key);
                auto root = insert(_root, new_node);
                if (_root == nullptr) {
                    _root = root;
                }
                fix_violation(_root, new_node);
                node = new_node;
            }
            new_item->next = node->list_items;
            node->list_items = new_item;
            node->size++;
        }

        [[nodiscard]] bool empty() const {
            return _size == 0;
        }

        [[nodiscard]] uint32_t size() const {
            return _size;
        }

        [[nodiscard]] uint32_t count(K key) const {
            auto node = search(_root, key);
            if (node == nullptr)
                return 0;
            return node->size;
        }

    private:
        bool walk(
                node_t* root,
                const walk_callback_t& callback) {
            if (root == nullptr) return false;
            walk(root->left, callback);
            if (!callback(root))
                return true;
            walk(root->right, callback);
            return false;
        }

        node_t* make_node(const K& key) {
            auto mem = _allocator->allocate(
                sizeof(node_t),
                alignof(node_t));
            auto node = new (mem) node_t();
            node->key = key;
            _size++;
            return node;
        }

        node_t* insert(node_t* root, node_t* pt) {
            if (root == nullptr)
                return pt;

            if (pt->key < root->key) {
                root->left = insert(root->left, pt);
                root->left->parent = root;
            } else if (pt->key > root->key) {
                root->right = insert(root->right, pt);
                root->right->parent = root;
            }

            return root;
        }

        node_t* search(node_t* root, K& key) const {
            if (root == nullptr || root->key == key)
                return root;

            if (root->key < key)
                return search(root->right, key);

            return search(root->left, key);
        }

        list_item_t* make_list_item(const V& value) {
            auto mem = _allocator->allocate(
                sizeof(list_item_t),
                alignof(list_item_t));
            auto list_item = new (mem) list_item_t();
            list_item->value = value;
            list_item->next = nullptr;
            return list_item;
        }

        void rotate_left(node_t*& root, node_t*& pt) {
            auto pt_right = pt->right;

            pt->right = pt_right->left;

            if (pt->right != nullptr)
                pt->right->parent = pt;

            pt_right->parent = pt->parent;

            if (pt->parent == nullptr)
                root = pt_right;

            else if (pt == pt->parent->left)
                pt->parent->left = pt_right;

            else
                pt->parent->right = pt_right;

            pt_right->left = pt;
            pt->parent = pt_right;
        }

        void clear_list_item(list_item_t* list_item) {
            auto current_item = list_item;
            while (current_item) {
                if constexpr (!std::is_trivially_destructible<V>::value)
                    current_item->~list_item_t();
                _allocator->deallocate(current_item);
                current_item = current_item->next;
            }
        }

        void rotate_right(node_t*& root, node_t*& pt) {
            auto pt_left = pt->left;

            pt->left = pt_left->right;

            if (pt->left != nullptr)
                pt->left->parent = pt;

            pt_left->parent = pt->parent;

            if (pt->parent == nullptr)
                root = pt_left;

            else if (pt == pt->parent->left)
                pt->parent->left = pt_left;

            else
                pt->parent->right = pt_left;

            pt_left->right = pt;
            pt->parent = pt_left;
        }

        void fix_violation(node_t*& root, node_t*& pt) {
            node_t* parent_pt = nullptr;
            node_t* grand_parent_pt = nullptr;

            while (pt != root
                   &&  pt->color != node_color_t::black
                   &&  pt->parent->color == node_color_t::red) {
                parent_pt = pt->parent;
                grand_parent_pt = pt->parent->parent;

                if (parent_pt == grand_parent_pt->left) {
                    auto uncle_pt = grand_parent_pt->right;

                    if (uncle_pt != nullptr
                    &&  uncle_pt->color == node_color_t::red) {
                        grand_parent_pt->color = node_color_t::red;
                        parent_pt->color = node_color_t::black;
                        uncle_pt->color = node_color_t::black;
                        pt = grand_parent_pt;
                    } else {
                        if (pt == parent_pt->right) {
                            rotate_left(root, parent_pt);
                            pt = parent_pt;
                            parent_pt = pt->parent;
                        }

                        rotate_right(root, grand_parent_pt);
                        std::swap(parent_pt->color, grand_parent_pt->color);
                        pt = parent_pt;
                    }
                } else {
                    auto uncle_pt = grand_parent_pt->left;

                    if (uncle_pt != nullptr
                    &&  uncle_pt->color == node_color_t::red) {
                        grand_parent_pt->color = node_color_t::red;
                        parent_pt->color = node_color_t::black;
                        uncle_pt->color = node_color_t::black;
                        pt = grand_parent_pt;
                    } else {
                        if (pt == parent_pt->left) {
                            rotate_right(root, parent_pt);
                            pt = parent_pt;
                            parent_pt = pt->parent;
                        }

                        rotate_left(root, grand_parent_pt);
                        std::swap(parent_pt->color, grand_parent_pt->color);
                        pt = parent_pt;
                    }
                }
            }

            root->color = node_color_t::black;
        }

    private:
        node_t* _root{};
        uint32_t _size{};
        memory::allocator_t* _allocator;
    };

}