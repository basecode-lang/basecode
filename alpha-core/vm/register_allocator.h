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

    struct register_allocator_t {
        register_allocator_t() {
            reset();
        }

        void reset() {
            used.clear();

            while (!available_float.empty())
                available_float.pop();

            while (!available_integer.empty())
                available_integer.pop();

            for (int8_t r = 63; r >= 0; r--) {
                available_integer.push(register_t {
                    .number = static_cast<registers_t>(r),
                    .type = register_type_t::integer
                });
                available_float.push(register_t {
                    .number = static_cast<registers_t>(r),
                    .type = register_type_t::floating_point
                });
            }
        }

        void free(const register_t& reg) {
            auto removed = used.erase(reg) > 0;
            if (removed) {
                if (reg.type == register_type_t::integer) {
                    available_integer.push(reg);
                } else {
                    available_float.push(reg);
                }
            }
        }

        bool allocate(register_t& reg) {
            if (reg.type == register_type_t::integer) {
                if (available_integer.empty())
                    return false;
                reg = available_integer.top();
                available_integer.pop();
            } else {
                if (available_float.empty())
                    return false;
                reg = available_float.top();
                available_float.pop();
            }
            used.insert(reg);
            return true;
        }

        std::stack<register_t> available_float {};
        std::stack<register_t> available_integer {};
        std::set<register_t, register_comparator> used {};
    };

};

