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
#include <basecode/memory/system.h>
#include <basecode/memory/object_pool.h>
#include <basecode/memory/slab_allocator.h>
#include <basecode/memory/trace_allocator.h>

namespace basecode {

    using namespace std::literals;
    using namespace basecode;
    using namespace basecode::memory;

    TEST_CASE("slab_allocator_t") {
        slab_allocator_t block1("block1"sv, 64);

        void* blocks[128];
        for (size_t i = 0; i < 128; i++) {
            blocks[i] = block1.allocate();
        }

        for (auto p : blocks)
            REQUIRE(p);
    }

    TEST_CASE("object_pool_t") {
        struct rect_t final {
            float x{}, y{};
            float w{}, h{};
        };

        struct point_t final {
            float x{}, y{};
        };

        object_pool_t pool{};

        auto r1 = pool.construct<rect_t>();
        REQUIRE(r1);
        r1->x = 1.0;
        r1->y = 1.0;
        r1->w = .5;
        r1->h = .5;

        auto p1 = pool.construct<point_t>();
        REQUIRE(p1);
        p1->x = 16.5;
        p1->y = -8.166;
    }

}