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
#include "terp.h"

namespace basecode::vm {

    struct allocation_status_t {
        bool live = false;
        registers_t reg {};
    };

    struct register_allocator_t {
        register_allocator_t() {
            reset();
        }

        void reset() {
            for (int8_t i = 0; i < 64; i++) {
                _ints[i] = allocation_status_t{false, static_cast<registers_t>(i)};
                _floats[i] = allocation_status_t{false, static_cast<registers_t>(i)};
            }
        }

        bool allocate(register_t& reg) {
            if (reg.type == register_type_t::integer) {
                auto index = move_to_next_available_int();
                _ints[index].live = true;
                reg.number = _ints[index].reg;
            } else {
                auto index = move_to_next_available_float();
                _floats[index].live = true;
                reg.number = _floats[index].reg;
            }
            return true;
        }

        void free(const register_t& reg) {
            if (reg.type == register_type_t::integer) {
                _ints[reg.number].live = false;
            } else {
                _floats[reg.number].live = false;
            }
        }

        size_t move_to_next_available_int() {
            size_t index = 0;
            while (_ints[index].live)
                index++;
            return index;
        }

        size_t move_to_next_available_float() {
            size_t index = 0;
            while (_floats[index].live)
                index++;
            return index;
        }

        allocation_status_t _ints[64];
        allocation_status_t _floats[64];
    };

};

