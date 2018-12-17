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
                move_to_next_available_int();
                _ints[_available_int].live = true;
                reg.number = _ints[_available_int].reg;
                _available_int++;
            } else {
                move_to_next_available_float();
                _floats[_available_float].live = true;
                reg.number = _floats[_available_float].reg;
                _available_float++;
            }
            return true;
        }

        void free(const register_t& reg) {
            if (reg.type == register_type_t::integer) {
                _ints[reg.number].live = false;
                if (reg.number < _available_int)
                    _available_int = reg.number;
            } else {
                _floats[reg.number].live = false;
                if (reg.number < _available_float)
                    _available_float = reg.number;
            }
        }

        void move_to_next_available_int() {
            while (_ints[_available_int].live)
                _available_int++;
        }

        void move_to_next_available_float() {
            while (_floats[_available_float].live)
                _available_float++;
        }

        size_t _available_int = 0;
        size_t _available_float = 0;
        allocation_status_t _ints[64];
        allocation_status_t _floats[64];
    };

};

