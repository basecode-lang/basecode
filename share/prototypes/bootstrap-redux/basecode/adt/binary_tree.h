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

#include <functional>
#include <initializer_list>
#include <basecode/memory/system.h>
#include <basecode/memory/allocator.h>

namespace basecode::adt {

    template <typename K>
    class binary_tree_t final {
    public:
        struct node_t final {
            K key;
            node_t* left{};
            node_t* right{};
        };

        using walk_callback_t = std::function<bool (node_t*)>;

        explicit binary_tree_t(
                memory::allocator_t* allocator = context::current()->allocator) : _allocator(allocator) {
            assert(_allocator);
        }

        binary_tree_t(
                std::initializer_list<K> elements,
                memory::allocator_t* allocator = context::current()->allocator) : _allocator(allocator) {
            assert(_allocator);
            insert(elements);
        }

        ~binary_tree_t() {
            auto i = 0;
            node_t* to_free[_size];
            walk(_root, [&](auto node) {
                to_free[i++] = node;
                return true;
            });
            to_free[i] = _root;

            for (auto node : to_free)
                _allocator->deallocate(node);
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

        node_t* root() {
            return _root;
        }

        node_t* insert(const K& key) {
            return insert(_root, key);
        }

        node_t* search(const K& key) {
            return search(_root, key);
        }

        [[nodiscard]] bool empty() const {
            return _size == 0;
        }

        node_t* search(node_t* root, const K& key) {
            if (root == nullptr || root->key == key)
                return root;

            if (root->key < key)
                return search(root->right, key);

            return search(root->left, key);
        }

        node_t* remove(node_t* root, const K& key) {
            if (root == nullptr)
                return nullptr;

            if (root->key > key) {
                root->left = remove(root->left, key);
                return root;
            } else if (root->key < key) {
                root->right = remove(root->right, key);
                return root;
            }

            if (root->left == nullptr) {
                auto temp = root->right;
                _allocator->deallocate(root);
                return temp;
            } else if (root->right == nullptr) {
                auto temp = root->left;
                _allocator->deallocate(root);
                return temp;
            } else {
                auto succ_parent = root->right;

                auto succ = root->right;
                while (succ->left) {
                    succ_parent = succ;
                    succ = succ->left;
                }

                succ_parent->left = succ->right;
                root->key = succ->key;
            }
        }

        node_t* insert(node_t* node, const K& key) {
            if (node == nullptr)
                return make_node(key);

            if (key < node->key)
                node->left = insert(node->left, key);
            else if (key > node->key)
                node->right = insert(node->right, key);

            return node;
        }

        [[nodiscard]] uint32_t size() const {
            return _size;
        }

    private:
        node_t* make_node(const K& key) {
            auto mem = _allocator->allocate(
                sizeof(node_t),
                alignof(node_t));

            auto node = new (mem) node_t();
            node->key = key;

            _size++;

            if (!_root)
                _root = node;

            return node;
        }

        void insert(std::initializer_list<K> elements) {
            node_t* root = nullptr;
            for (const auto& k : elements)
                root = insert(root, k);
        }

    private:
        node_t* _root{};
        uint32_t _size{};
        memory::allocator_t* _allocator;
    };

}