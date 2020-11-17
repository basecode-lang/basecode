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

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>
#include <basecode/defer.h>
#include <basecode/signals/hook.h>
#include <basecode/memory/system.h>
#include <basecode/errors/errors.h>
#include <basecode/context/context.h>
#include <basecode/logging/fake_logger.h>
#include <basecode/memory/trace_allocator.h>

#define TRACE_ALLOCATOR 0
using namespace basecode;

int main(int argc, char** argv) {
    int rc = 0;

    memory::initialize();
    defer({
        memory::shutdown();
        context::shutdown();
    });

    {
        memory::allocator_t* default_allocator{};
#if TRACE_ALLOCATOR
        memory::trace_allocator_t tracer(memory::default_scratch_allocator());
        default_allocator = &tracer;
#else
        default_allocator = memory::default_scratch_allocator();
#endif
        logging::fake_logger_t fake_logger(default_allocator);
        context::initialize(default_allocator, &fake_logger);

        result_t r;
        if (!errors::initialize(r)) return 1;
        if (!signals::initialize(r)) return 1;

        rc = Catch::Session().run(argc, argv);

        errors::shutdown();
        signals::shutdown();
    }

    return (rc < 0xff ? rc : 0xff);
}
