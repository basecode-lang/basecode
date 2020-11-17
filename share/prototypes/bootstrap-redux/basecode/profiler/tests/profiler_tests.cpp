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

#include <thread>
#include <basecode/defer.h>
#include <catch2/catch.hpp>
#include <basecode/logging/logger.h>
#include <basecode/profiler/timer.h>

namespace basecode {

    using namespace std::literals;

    static uint64_t fib(uint64_t n) {
        if (n < 2) return n;
        return fib(n - 2) + fib(n - 1);
    }

    TEST_CASE("profiler::timer_t") {
        auto logger = context::current()->logger;

        result_t r{};
        REQUIRE(profiler::initialize(r));

        profiler::timer_t timer{};
        auto start = std::chrono::high_resolution_clock::now();
        timer.start();
        fib(20);
        timer.stop();
        auto end = std::chrono::high_resolution_clock::now();

        const auto ns_timer_elapsed = timer.elapsed();
        logger->info("timer.elapsed() = {}", ns_timer_elapsed);
        REQUIRE(ns_timer_elapsed > 0);

        const auto high_res_clock_elapsed = end - start;
        logger->info("high_rest_clock_elapsed = {}", high_res_clock_elapsed.count());
    }

}
