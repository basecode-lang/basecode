// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <set>
#include <stack>
#include <cstdint>
#include "vm_types.h"

namespace basecode::vm {

    struct allocation_status_t {
        bool live = false;
        registers_t reg {};
    };

    class register_allocator {
    public:
        register_allocator();

        void reset();

        bool allocate(register_t& reg);

        void release(const register_t& reg);

    private:
        size_t move_to_next_available_int();

        size_t move_to_next_available_float();

    private:
        allocation_status_t _ints[64];
        allocation_status_t _floats[64];
    };

};

