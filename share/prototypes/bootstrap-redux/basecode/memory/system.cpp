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

#include <boost/interprocess/mapped_region.hpp>
#include "system.h"
#include "dl_allocator.h"
#include "scratch_allocator.h"

namespace basecode::memory {

    struct globals_t final {
        static const int allocator_memory_size =
            sizeof(dl_allocator_t) + sizeof(scratch_allocator_t);

        uint8_t buffer[allocator_memory_size]{};

        size_t os_page_size{};
        dl_allocator_t* default_allocator = nullptr;
        scratch_allocator_t* default_scratch_allocator = nullptr;
    };

    globals_t g_globals{};

    void shutdown() {
        g_globals.default_scratch_allocator->~scratch_allocator_t();
        g_globals.default_allocator->~dl_allocator_t();
        g_globals = globals_t();
    }

    size_t os_page_size() {
        return g_globals.os_page_size;
    }

    allocator_t* default_allocator() {
        return g_globals.default_allocator;
    }

    allocator_t* default_scratch_allocator() {
        return g_globals.default_scratch_allocator;
    }

    void initialize(uint32_t scratch_buffer_size) {
        g_globals.os_page_size = boost::interprocess::mapped_region::get_page_size();
        uint8_t* p = g_globals.buffer;
        g_globals.default_allocator = new (p) dl_allocator_t();
        p += sizeof(dl_allocator_t);
        g_globals.default_scratch_allocator = new (p) scratch_allocator_t(
            scratch_buffer_size,
            g_globals.default_allocator);
    }

}
