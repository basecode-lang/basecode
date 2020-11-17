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

#include <catch2/catch.hpp>
#include <basecode/types.h>
#include <basecode/adt/set.h>
#include <basecode/adt/stack.h>
#include <basecode/adt/trie_map.h>
#include <basecode/adt/multi_map.h>
#include <basecode/adt/binary_tree.h>
#include <basecode/adt/red_black_tree.h>

namespace basecode {

    using namespace std::literals;

    struct range_t final {
        int32_t begin;
        int32_t end;

        bool operator<(const range_t& rhs) const {
            return rhs.end < begin;
        }

        bool operator>(const range_t& rhs) const {
            return rhs.end > begin;
        }

        bool operator==(const range_t& rhs) const {
            return rhs.begin >= begin && rhs.end <= end;
        }
    };

    using namespace basecode::adt;

    TEST_CASE("set_t with small initializer list") {
        set_t<int32_t> set_of_integers{0, 1, 2, 3, 4, 5, 6, 7, 8};

        REQUIRE(!set_of_integers.empty());
        REQUIRE(set_of_integers.size() == 9);

        REQUIRE(set_of_integers.count(1) == 1);
        REQUIRE(set_of_integers.has(6));
        REQUIRE(!set_of_integers.has(10));

        REQUIRE(set_of_integers.remove(3));
        REQUIRE(!set_of_integers.has(3));
        REQUIRE(set_of_integers.count(3) == 0);

        for (size_t i = 10; i < 100; i++)
            set_of_integers.insert(i);

        REQUIRE(set_of_integers.size() == 98);

        auto elements = set_of_integers.elements();
        std::sort(
            std::begin(elements),
            std::end(elements),
            [](auto lhs, auto rhs) {
                return lhs < rhs;
            });

        REQUIRE(elements.size() == 98);
    }

    TEST_CASE("array_t with small initializer list") {
        array_t<int32_t> numbers{0, 1, 2, 3, 4, 5, 6, 7, 8};

        REQUIRE(!numbers.empty());
        REQUIRE(numbers.size() == 9);

        int32_t k = 0;
        for (auto n : numbers)
            REQUIRE(n == k++);
    }

    TEST_CASE("array_t insert") {
        array_t<int32_t> numbers{0, 1, 2, 3, 4, 5, 6, 7, 8};

        REQUIRE(!numbers.empty());
        REQUIRE(numbers.size() == 9);

        auto v = *numbers.insert(std::begin(numbers) + 3, 30);
        REQUIRE(v == 30);

        REQUIRE(numbers.size() == 10);

        int32_t k = 0;
        int32_t j = 0;
        for (auto n : numbers) {
            if (j == 3) {
                REQUIRE(n == 30);
            } else {
                REQUIRE(n == k++);
            }
            ++j;
        }
    }

    TEST_CASE("array_t erase") {
        array_t<int32_t> numbers{0, 1, 2, 3, 4, 5, 6, 7, 8};

        REQUIRE(!numbers.empty());
        REQUIRE(numbers.size() == 9);

        auto v = *numbers.erase(std::begin(numbers) + 3);
        REQUIRE(v == 4);

        REQUIRE(numbers.size() == 8);

        int32_t k = 0;
        int32_t j = 0;
        for (auto n : numbers) {
            if (j == 3) {
                REQUIRE(n == 4);
                k = n + 1;
            } else {
                REQUIRE(n == k++);
            }
            ++j;
        }
    }

    TEST_CASE("array_t reserve space; fill") {
        array_t<int32_t> numbers;
        numbers.reserve(4096);

        REQUIRE(numbers.empty());
        REQUIRE(numbers.size() == 0);
        REQUIRE(numbers.capacity() == 4096);

        for (size_t i = 0; i < 4096; i++)
            numbers.add(i);

        REQUIRE(!numbers.empty());
        REQUIRE(numbers.size() == 4096);
        REQUIRE(numbers.capacity() == 4096);

        std::sort(
            std::begin(numbers),
            std::end(numbers),
            [](auto lhs, auto rhs) {
                return lhs > rhs;
            });

        for (size_t i = 0; i < 4096; i++)
            REQUIRE(numbers[i] == 4095 - i);
    }

    TEST_CASE("stack_t basics") {
        stack_t<int32_t> numbers{0, 1, 2, 3, 4, 5, 6, 7, 8};

        REQUIRE(!numbers.empty());
        REQUIRE(numbers.depth() == 9);

        auto n = 8;
        while (!numbers.empty()) {
            auto top = *numbers.top();
            REQUIRE(top == n--);
            numbers.pop();
        }

        numbers.push(100);
        numbers.push(200);
        numbers.push(300);
        REQUIRE(numbers.depth() == 3);

        n = 300;
        while (!numbers.empty()) {
            auto top = *numbers.top();
            REQUIRE(top == n);
            n -= 100;
            numbers.pop();
        }

        numbers.clear();
        REQUIRE(numbers.empty());
        REQUIRE(numbers.depth() == 0);
    }

    TEST_CASE("bst_t basics") {
        binary_tree_t tree{50, 30, 20, 40, 70, 60, 80};

        REQUIRE(!tree.empty());
        REQUIRE(tree.size() == 7);

        auto node = tree.search(40);
        REQUIRE(node);
        REQUIRE(node->key == 40);
    }

    TEST_CASE("bst_t with custom range") {
        binary_tree_t<range_t> tree;

        tree.insert(range_t{1, 10});
        tree.insert(range_t{20, 33});
        tree.insert(range_t{34, 61});
        tree.insert(range_t{62, 110});
        tree.insert(range_t{111, 186});
        tree.insert(range_t{187, 222});
        tree.insert(range_t{11, 19});

        auto found1 = tree.search(range_t{12, 12});
        REQUIRE(found1);
        REQUIRE(found1->key.begin == 11);
        REQUIRE(found1->key.end == 19);

    }

    TEST_CASE("red_black_tree_t with custom range") {
        red_black_tree_t<range_t> tree;

        tree.insert(range_t{1, 10});
        tree.insert(range_t{20, 33});
        tree.insert(range_t{34, 61});
        tree.insert(range_t{62, 110});
        tree.insert(range_t{111, 186});
        tree.insert(range_t{187, 222});
        tree.insert(range_t{11, 19});

        auto found1 = tree.search(range_t{202, 202});
        REQUIRE(found1);
        REQUIRE(found1->key.begin == 187);
        REQUIRE(found1->key.end == 222);
    }

    TEST_CASE("multi_map_t basics") {
        multi_map_t<string_t, int32_t> map;
        const auto key1 = "key1"sv;
        const auto key2 = "key2"sv;

        map.insert(key1, 1);
        map.insert(key1, 2);
        map.insert(key1, 3);

        REQUIRE(map.count(key1) == 3);
        REQUIRE(map.size() == 1);

        map.clear();
        REQUIRE(map.size() == 0);

        map.insert(key1, 1);
        map.insert(key1, 2);
        map.insert(key2, 3);
        map.insert(key2, 4);

        REQUIRE(map.size() == 2);
    }

    TEST_CASE("multi_map_t items") {
        multi_map_t<string_t, int32_t> map;

        const auto key1 = "key1"sv;

        map.insert(key1, 1);
        map.insert(key1, 2);
        map.insert(key1, 3);

        auto items = map.find(key1);
        REQUIRE(items.size() == 3);
        REQUIRE(*items[0] == 3);
        REQUIRE(*items[1] == 2);
        REQUIRE(*items[2] == 1);
    }

    TEST_CASE("trie_map_t") {
        trie_map_t<int> map{};

        map.insert("the", 1);
        map.insert("a", 2);
        map.insert("there", 3);
        map.insert("answer", 4);
        map.insert("any", 5);
        map.insert("by", 6);
        map.insert("bye", 7);
        map.insert("their", 8);

        const auto& pairs = map.pairs();
        REQUIRE(pairs.size() == 8);
    }

}