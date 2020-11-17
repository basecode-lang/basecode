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
#include <functional>
#include <initializer_list>
#include <basecode/memory/system.h>
#include <basecode/memory/allocator.h>

namespace basecode::adt {

    template <typename K>
    class red_black_tree_t final {
    public:
        enum class node_color_t : uint8_t {
            red,
            black
        };

        struct node_t final {
            K key;
            node_t* left{};
            node_t* right{};
            node_t* parent{};
            node_color_t color{};
        };

        using walk_callback_t = std::function<bool (node_t*)>;

        explicit red_black_tree_t(
            memory::allocator_t* allocator = context::current()->allocator) : _allocator(allocator) {
            assert(_allocator);
        }

        red_black_tree_t(
                std::initializer_list<K> elements,
                memory::allocator_t* allocator = context::current()->allocator) : _allocator(allocator) {
            assert(_allocator);
            insert(elements);
        }

        ~red_black_tree_t() {
            clear();
        }

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

        void clear() {
            auto i = 0;
            node_t* to_free[_size];
            walk(_root, [&](auto node) {
                to_free[i++] = node;
                return true;
            });
            to_free[i] = _root;

            for (auto node : to_free) {
                if constexpr (!std::is_trivially_destructible<K>::value)
                    node->~node_t();
                _allocator->deallocate(node);
            }

            _size = 0;
            _root = nullptr;
        }

        node_t* root() {
            return _root;
        }

        void insert(const K& key) {
            auto new_node = make_node(key);
            auto root = insert(_root, new_node);
            if (_root == nullptr)
                _root = root;
            fix_violation(_root, new_node);
        }

        node_t* search(const K& key) {
            return search(_root, key);
        }

        [[nodiscard]] bool empty() const {
            return _size == 0;
        }

        [[nodiscard]] uint32_t size() const {
            return _size;
        }

        node_t* search(node_t* root, const K& key) {
            if (root == nullptr || root->key == key)
                return root;

            if (root->key < key)
                return search(root->right, key);

            return search(root->left, key);
        }

    private:
        node_t* make_node(const K& key) {
            auto mem = _allocator->allocate(sizeof(node_t), alignof(node_t));
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

        void insert(std::initializer_list<K> elements) {
            node_t* root = nullptr;
            for (const auto& k : elements)
                root = insert(root, k);
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