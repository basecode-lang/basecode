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
                available_float.push(static_cast<registers_t>(r));
                available_integer.push(static_cast<registers_t>(r));
            }
        }

        bool allocate(register_t& reg) {
            if (reg.type == register_type_t::integer) {
                if (available_integer.empty())
                    return false;
                auto next_reg = available_integer.top();
                reg.number = next_reg;
                available_integer.pop();
            } else {
                if (available_float.empty())
                    return false;
                auto next_reg = available_float.top();
                reg.number = next_reg;
                available_float.pop();
            }
            used.insert(register_index(reg.number, reg.type));
            return true;
        }

        void free(const register_t& reg) {
            auto removed = used.erase(register_index(reg.number, reg.type)) > 0;
            if (removed) {
                if (reg.type == register_type_t::integer) {
                    available_integer.push(reg.number);
                } else {
                    available_float.push(reg.number);
                }
            }
        }

        std::set<size_t> used {};
        std::stack<registers_t> available_float {};
        std::stack<registers_t> available_integer {};
    };

};

